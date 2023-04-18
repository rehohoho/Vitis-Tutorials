#ifndef MULT_H_
#define MULT_H_

#include <stdint.h>

#define V_LEN 		512
#define V_CHUNKS 	V_LEN/8

void scalar_mul(
	input_window<int32>* __restrict data1,
	input_window<int32>* __restrict data2,
	output_window<int32>* __restrict out
);

void vector_mul (
	input_window<int32>* __restrict data1,
	input_window<int32>* __restrict data2,
	output_window<int32>* __restrict out
);

#endif // CORE_01_H
