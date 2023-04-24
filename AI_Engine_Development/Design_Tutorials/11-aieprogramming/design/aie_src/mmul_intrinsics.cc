#include "mmul_intrinsics.h"
#include "kernel_utils.h"


template <int VMULM, int VMULN>
void vmul_intrinsic_scalar(
  input_window<int16>* matA, // column-major
  input_window<int16>* matB, // single row
  output_window<int16>* matC
) {
  PROFILE_HEADER;

  for (int i = 0; i < VMULM; i++) {
    int res = 0;
    for (int j = 0; j < VMULN; j++) {
      int16 a = window_read(matA);
      int16 b = window_readincr(matB);
      res += a * b; // matB is a circular buffer
      window_incr(matA, VMULM); // column major
    }
    window_writeincr(matC, (int16_t) res);
    window_incr(matA, 1); // next row
  }

  PROFILE_FOOTER;
}


/*
mac16 is 16 lane of 2 muls each

acc0 += z00*x00 + z01*x01
acc1 += z10*x10 + z11*x11
acc2 += z20*x20 + z21*x21
...
acc15 += z150*x150 + z151*x151
*/
template <int VMULM, int VMULN>
void vmul_intrinsic_vector(
  input_window<int16>* matA, // column-major
  input_window<int16>* matB, // single row
  output_window<int16>* matC
) {
	PROFILE_HEADER;

  v16int16 buf_matB = window_read_v16(matB);
	v16acc48 acc = null_v16acc48(); 

	for (unsigned int i = 0; i < VMULM / 16; i++) { // compute 16 outputs per i
		acc = null_v16acc48();
		for(int j = 0; j < 16; j += 2) {
      v32int16 buf_matA = undef_v32int16();       // read 2 rows of matA for this col block
      buf_matA = upd_w(buf_matA, 0, window_read_v16(matA));
      window_incr(matA, 64);
      buf_matA = upd_w(buf_matA, 1, window_read_v16(matA));
      int incr_num = (j == 14) ? 80 : 64;         // next col block: 64+16 if last iter
      window_incr(matA, incr_num);
      
      // multiply by 2 coefficients
      // acc, xbuff, xstart, xoffsets, xoffsets_hi, xsquare, zbuff, zstart, zoffsets, zoffsets_hi, zstep
      // xoffsets: 4b offset for lane 0,2,4,6, for 70, off0=2*0, off2=(7+0+1)*2
      acc = mac16(acc, buf_matA, 0, 0x73727170, 0x77767574, 0x3120, buf_matB, j, 0x0, 0x0, 1);
		}
		window_writeincr(matC, srs(acc, 0)); // bitshift from acc46 to int16
	}

  PROFILE_FOOTER;
}


// restrict necessary to remove pointer aliasing dependenncies when loading buf_matA
template <int VMULM, int VMULN>
void vmul_intrinsic_vector_2matA(
  input_window<int16>* __restrict matA1, // column-major
  input_window<int16>* __restrict matA2, // column-major
  input_window<int16>* __restrict matB, // single row
  output_window<int16>* __restrict matC
) {
	PROFILE_HEADER;

  v16int16 buf_matB = window_read_v16(matB);
	v16acc48 acc = null_v16acc48(); 

	for (unsigned int i = 0; i < VMULM / 16; i++) 
    chess_prepare_for_pipelining
  { // compute 16 outputs per i
		acc = null_v16acc48();
		for(int j = 0; j < 16; j += 2) {
      int incr_num = (j == 14) ? 80 : 64;         // next col block: 64+16 if last iter
      v32int16 buf_matA = undef_v32int16();       // read 2 rows of matA for this col block
      buf_matA = upd_w(buf_matA, 0, window_read_v16(matA1));
      window_incr(matA1, incr_num);
      buf_matA = upd_w(buf_matA, 1, window_read_v16(matA2));
      window_incr(matA2, incr_num);
      
      // multiply by 2 coefficients
      // acc, xbuff, xstart, xoffsets, xoffsets_hi, xsquare, zbuff, zstart, zoffsets, zoffsets_hi, zstep
      // xoffsets: 4b offset for lane 0,2,4,6, for 70, off0=2*0, off2=(7+0+1)*2
      acc = mac16(acc, buf_matA, 0, 0x73727170, 0x77767574, 0x3120, buf_matB, j, 0x0, 0x0, 1);
		}
		window_writeincr(matC, srs(acc, 0)); // bitshift from acc46 to int16
	}

  PROFILE_FOOTER;
}


template <int VMULM, int VMULK, int VMULN>
void mmul_intrinsic_scalar(
  input_window<int16>* matA, // column-major
  input_window<int16>* matB, // column-major
  output_window<int16>* matC // column-major
) {
  PROFILE_HEADER;

  for (int i = 0; i < VMULM; i++) {
    for (int j = 0; j < VMULN; j++) {
      int res = 0;
      for (int k = 0; k < VMULK; k++) {
        int16 a = window_read(matA);
        int16 b = window_readincr(matB);
        res += a * b; // matB is a circular buffer
        window_incr(matA, VMULM);
      }    
      window_write(matC, (int16_t) res);
      window_incr(matC, VMULM);
    }
    window_incr(matA, 1); // next row
    window_incr(matC, 1); // next row
  }

  PROFILE_FOOTER;
}