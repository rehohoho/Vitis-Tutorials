#include "mmul.h"
#include "kernel_utils.h"


template <int ROWA, int COLA, int COLB, int SHIFT>
void MmulAieapi<ROWA, COLA, COLB, SHIFT>::filter(
	input_window<int8> * __restrict matA, // in 4*16 block order (each block as a vector), block placed row order
	input_window<int8> * __restrict matB, // in 16*8 block order (each block as a vector), block placed col order
	output_window<int8> * __restrict matC
) {
  using MMUL = aie::mmul<M, K, N, int8, int8>;
  const int8* __restrict pA = (int8*) matA->ptr;
  const int8* __restrict pB = (int8*) matB->ptr;
  int8* __restrict pC = (int8*) matC->ptr;

  PROFILE_HEADER;

  for (unsigned i = 0; i < num_rowA; i++) { // A's row block #
    for (unsigned j = 0; j < num_colB; j++) { // B's col block #
      // row offset: i * num_colA * size_A = number of A blocks * size of A blocks (M*K)
      // each i completes one row of blocks
			const int8 * __restrict pA1 = pA + ( i * num_colA + 0) * MMUL::size_A;
      // col offset: j * size_B = number of B cols done * size of B blocks (K*M)
      // each j completes one column of blocks
			const int8 * __restrict pB1 = pB + ( 0 * num_colB + j) * MMUL::size_B;

      MMUL C00; 

			// load first instance
			aie::vector<int8, MMUL::size_A> A0 = aie::load_v<MMUL::size_A>(pA1); pA1 += MMUL::size_A;
      aie::vector<int8, MMUL::size_B> B0 = aie::load_v<MMUL::size_B>(pB1); pB1 += MMUL::size_B * num_colB;
      C00.mul(A0, B0);

      // compute for row of blocks in A and col of blocks in B
			for (unsigned k = 0; k < num_colA-1; k++) {
        A0 = aie::load_v<MMUL::size_A>(pA1); pA1 += MMUL::size_A;
        B0 = aie::load_v<MMUL::size_B>(pB1); pB1 += MMUL::size_B * num_colB;
        C00.mac(A0, B0);
      }

      aie::store_v(pC, C00.template to_vector<int8>(SHIFT));
			pC += MMUL::size_C;
    }
  }

  PROFILE_FOOTER;
}


template <int ROWA, int COLA, int COLB, int SHIFT>
void mmul_aieapi_shuffleA (
	input_window<int8>* __restrict matA, 
	output_window<int8>* __restrict matAout
) {
  const int sizeA = M * K;
  auto pV = aie::begin_vector<16>((int8*) matA->ptr);
  auto pOut = aie::begin_vector<sizeA>((int8*) matAout->ptr);
  aie::vector<int8, sizeA> mm;

  // outputs row blocks, indexed via row blocks
  for (int i = 0; i< ROWA / M; i++) {     // generate one row block per i
    for (int j = 0; j < COLA / K; j++) {  // generate one block per j, place row order
      for (int k = 0; k < M; k++) {       // generate one row of block per k, place row order
        mm.insert(k, *pV);  // insert each row of block in mm
        pV = pV + 4;        // next row: 4*16=64 
      }
      *pOut++ = mm; // append to pOut: in mm row order
      pV = pV - 15; // next block: -(4+4+4+4-1)*16 = -64-64-64-64+16
      
    }
    pV = pV + 12; // next row of blocks: -(4+16)*16 = -64 + 4*64
  }
}


template <int ROWA, int COLA, int COLB, int SHIFT>
void mmul_aieapi_shuffleB (
	input_window<int8> * __restrict matB, 
	output_window<int8> * __restrict matBout
) {
  const int sizeA = K * N;
  auto pV = aie::begin_vector<16>((int8*) matB->ptr);
  auto pOut = aie::begin_vector<16>((int8*) matBout->ptr);
  aie::vector<int8, 16> sv1, sv2;

  // outputs col blocks, indexed via col blocks
  for (int i = 0; i < COLB / K; i++) {        // generate output col block per i
    for (int j = 0; j < COLA / N / 2; j++) {  // generate two blocks per j (row-order) due to vector<16>, place row-order
      
      /** for each k: generate rows for two consecutive blocks
       * 
       *         8        8
       * sv1: aaaaaaaa aaaaaaaa  
       * sv2: bbbbbbbb bbbbbbbb  
       *      cccccccc cccccccc 8*2=16
       *      dddddddd dddddddd
       * ...
       * 
       *              8x8
       * aaaaaaaabbbbbbbccccccccdddddddd ...
       * ...
       * aaaaaaaabbbbbbbccccccccdddddddd ...  4
       * ...
       */
      for(int k = 0; k < K / 2; k++) { // generate two rows of two blocks per k due to interleaving sv1+sv2, place row-order
        sv1 = *pV; pV = pV + 4; // next row: 4*16=64
        sv2 = *pV; pV = pV + 4; // next row: 4*16=64
        
        // alternate 8 elements from sv1 then sv2, outputs pair of vectors
        auto mm = aie::interleave_zip(sv1, sv2, 8);
        *pOut = mm.first; pOut += 8;  // append to pOut, go next block: 8*16=128
        *pOut = mm.second; pOut -= 7; // append to pOut next block, go back to first block
      }
      pOut += 8; // next block: completed two block, ended off at one block
      pV -= 63;  // next col of blocks: (-64+1)*16 = -64*16 + 16 
    }
    pV += 60; // (-4+64)
  }
}


template <int ROWA, int COLA, int COLB, int SHIFT>
void mmul_aieapi_shuffleC(
	input_window<int8> * __restrict matC, 
	output_window<int8> * __restrict matCout
) {
  const int sizeA = M*N;
  auto pV = aie::begin_vector<sizeA>((int8*) matC->ptr);
  auto pOut = aie::begin_vector<sizeA>((int8*) matCout->ptr);

  aie::vector<int8, sizeA> mm1, mm2, mm3, mm4;
  for(int i = 0; i < ROWA / M; i++) {       // 
    for(int j = 0; j< COLA / N / 4; j++) {  // 
      mm1 = *pV++;
      mm2 = *pV++;
      mm3 = *pV++;
      mm4 = *pV++;
      auto mm12 = aie::interleave_zip(mm1, mm2, 8);
      auto mm34 = aie::interleave_zip(mm3, mm4, 8);
      auto mm1234_low = aie::interleave_zip(mm12.first, mm34.first, 16);
      auto mm1234_high = aie::interleave_zip(mm12.second, mm34.second, 16);
      *pOut = mm1234_low.first; pOut = pOut + 2;
      *pOut = mm1234_low.second; pOut = pOut + 2;
      *pOut = mm1234_high.first; pOut = pOut + 2;
      *pOut = mm1234_high.second; pOut = pOut - 5;
    }
    pOut = pOut + 6;
  }
}