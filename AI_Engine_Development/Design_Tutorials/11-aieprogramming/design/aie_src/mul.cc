#include <stdlib.h>
#include <stdio.h>
#include "aie_api/aie.hpp"

#include "mul.h"


// docs: 1053 cycles
void scalar_mul(
	input_window<int32>* __restrict data1,
	input_window<int32>* __restrict data2,
	output_window<int32>* __restrict out
) {
	for (int i = 0; i < V_LEN; i++) {
		int32 a = window_readincr(data1);
		int32 b = window_readincr(data2);
		int32 c = a * b;
		window_writeincr(out, c);
	}
}

// docs: 93 cycles
// 8x throughput from vectorizing, loop optimization for more
// maximum performance: 32 MAC/instruction for 16-bit vectors => 32GMAC/s
void vector_mul (
	input_window<int32>* __restrict data1, // __restrict for better compiler optimization
	input_window<int32>* __restrict data2,
	output_window<int32>* __restrict out
) {
	for (int i = 0; i < V_CHUNKS; i++) chess_prepare_for_pipelining {	//optimize pipeline for loop
		aie::vector<int32, 8> va = window_readincr_v<8>(data1); // reads vector of 8 int32s
		aie::vector<int32, 8> vb = window_readincr_v<8>(data2);
		aie::accum<acc80, 8> vc = aie::mul(va, vb);
		window_writeincr(out, vc.to_vector<int32>(0)); // shift round saturate 
	}
}