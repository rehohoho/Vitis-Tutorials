#include "mmul.h"
#include "kernel_utils.h"


template <int ROWA, int COLA, int COLB, int SHIFT>
void MmulAieapi<ROWA, COLA, COLB, SHIFT>::filter(
	input_window<int8> * __restrict matA, // shuffled: blocks are in row order within row blocks
	input_window<int8> * __restrict matB, // shuffled: blocks are in row order within col blocks
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
			const int8 * __restrict pA1 = pA + ( i * num_colA + 0) * MMUL::size_A;
      // col offset: j * size_B = number of B cols done * size of B blocks (K*M)
			const int8 * __restrict pB1 = pB + ( 0 * num_colB + j) * MMUL::size_B;

      MMUL C00; 

			// load first instance
			aie::vector<int8, MMUL::size_A> A0 = aie::load_v<MMUL::size_A>(pA1); pA1 += MMUL::size_A;
      aie::vector<int8, MMUL::size_B> B0 = aie::load_v<MMUL::size_B>(pB1); pB1 += MMUL::size_B * num_colB;
      C00.mul(A0, B0);

      // compute for A's row block and B's col block
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
