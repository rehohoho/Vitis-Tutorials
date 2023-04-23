#include "mmul_intrinsics.h"
#include "kernel_utils.h"


template <int VMULM, int VMULN>
void vmul_intrinsic_scalar(
  input_window<int16>* matA, // column-major
  input_window<int16>* matB, // single row
  output_window<int16>* matC
) {
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
}
