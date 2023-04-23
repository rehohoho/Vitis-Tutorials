#ifndef MMUL_INTRINSICS_H_
#define MMUL_INTRINSICS_H_

#include <adf.h>
#include "aie_api/aie.hpp"


template <int VMULM, int VMULN>
void vmul_intrinsic_scalar(
	input_window<int16>* matA,
  input_window<int16>* matB,
  output_window<int16>* matC);

#endif // MMUL_INTRINSICS_H_
