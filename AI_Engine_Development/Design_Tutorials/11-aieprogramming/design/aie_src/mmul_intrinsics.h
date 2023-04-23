#ifndef MMUL_INTRINSICS_H_
#define MMUL_INTRINSICS_H_

#include <adf.h>
#include "aie_api/aie.hpp"


template <int VMULM, int VMULN>
void vmul_intrinsic_scalar(
	input_window<int16>* matA,  // column-major
  input_window<int16>* matB,  // single-row
  output_window<int16>* matC);

template <int VMULM, int VMULN>
void vmul_intrinsic_vector(
	input_window<int16>* matA,  // column-major
  input_window<int16>* matB,  // single row
  output_window<int16>* matC);

template <int VMULM, int VMULN>
void vmul_intrinsic_vector_2matA(
  input_window<int16>* matA1, // column-major
  input_window<int16>* matA2, // column-major
  input_window<int16>* matB,  // single row
  output_window<int16>* matC);

#endif // MMUL_INTRINSICS_H_
