#include "fir_intrinsics.h"


/**
 * mul4:		y0 += d1t0 d2t1,
 * 					y1 += 		 d2t0 d3t1,
 * 					y2 += 				  d3t0 d4t1,
 * 					y3 += 						   d4t0 d5t1
 * 
 * offset 0x3210 with start 1, gives d1, d2, d3, d4 starting index
 * offset 0x0000 with start 0, gives t0, t0, t0, t0 starting index
 * 
 * mac4:		y0 += 					d3t0 d4t1
 * 					y1 +=								 d4t0 d5t1
 * 					y2 += 										d5t0 d6t1
 * 					y3 += 												 d6t0 d7t1
 * ...
 * ...
 */
#define MULMAC(N) \
		taps =  *coeff++; \
		acc = mul4(data,N,0x3210,1,taps,0,0x0000,1); \
		acc = mac4(acc,data,N+2,0x3210,1,taps,2,0x0000,1);\
		acc = mac4(acc,data,N+4,0x3210,1,taps,4,0x0000,1);\
		acc = mac4(acc,data,N+6,0x3210,1,taps,6,0x0000,1)

#define MACMAC(N) \
		taps =  *coeff++; \
		acc = mac4(acc,data,N,0x3210,1,taps,0,0x0000,1); \
		acc = mac4(acc,data,N+2,0x3210,1,taps,2,0x0000,1);\
		acc = mac4(acc,data,N+4,0x3210,1,taps,4,0x0000,1);\
		acc = mac4(acc,data,N+6,0x3210,1,taps,6,0x0000,1)

// Compute 32/32 partial results for 32 samples
template <int SAMPLES, int SHIFT>
void Vector_32tap_fir_intrinsics<SAMPLES, SHIFT>::filter(
  input_stream_cint16* sin,
  output_stream_cint16* sout
) {
  // For profiling only 
  unsigned cycle_num[2];
  aie::tile tile = aie::tile::current();
  cycle_num[0] = tile.cycles(); // cycle counter of the AI Engine tile

	v8cint16 *coeff = (v8cint16*) weights;
	v8cint16 taps = undef_v8cint16();
	v32cint16 *ptr_delay_line = (v32cint16*) delay_line;
	v32cint16 data = *ptr_delay_line;

	v4cacc48 acc = undef_v4cacc48();

	for(int i = 0; i < SAMPLES / 32; i++)
		chess_prepare_for_pipelining
		chess_loop_range(SAMPLES/32, SAMPLES/32)
		chess_pipeline_adjust_preamble(10)
	{
		// loop rotation: start on first index to load for next iteration near end to reduce code dependency
		MULMAC(0);	// 4 ops on 4 samples
		MACMAC(8);
		MACMAC(16);
		data = upd_v(data, 0, readincr_v4(sin));
		MACMAC(24);
		writeincr_v4(sout, srs(acc, SHIFT));
		coeff -= 4;

		MULMAC(4);
		MACMAC(12);
		MACMAC(20);
		data = upd_v(data, 1, readincr_v4(sin));
		MACMAC(28);
		writeincr_v4(sout, srs(acc, SHIFT));
		coeff -= 4;

		MULMAC(8);
		MACMAC(16);
		MACMAC(24);
		data = upd_v(data, 2, readincr_v4(sin));
		MACMAC(0);
		writeincr_v4(sout, srs(acc, SHIFT));
		coeff -= 4;

		MULMAC(12);
		MACMAC(20);
		MACMAC(28);
		data = upd_v(data, 3, readincr_v4(sin));
		MACMAC(4);
		writeincr_v4(sout, srs(acc, SHIFT));
		coeff -= 4;

		MULMAC(16);
		MACMAC(24);
		MACMAC(0);
		data = upd_v(data, 4, readincr_v4(sin));
		MACMAC(8);
		writeincr_v4(sout, srs(acc, SHIFT));
		coeff -= 4;

		MULMAC(20);
		MACMAC(28);
		MACMAC(4);
		data = upd_v(data, 5, readincr_v4(sin));
		MACMAC(12);
		writeincr_v4(sout, srs(acc, SHIFT));
		coeff -= 4;

		MULMAC(24);
		MACMAC(0);
		MACMAC(8);
		data = upd_v(data, 6, readincr_v4(sin));
		MACMAC(16);
		writeincr_v4(sout, srs(acc, SHIFT));
		coeff -= 4;

		MULMAC(28);
		MACMAC(4);
		MACMAC(12);
		data = upd_v(data, 7, readincr_v4(sin));
		MACMAC(20);
		writeincr_v4(sout, srs(acc, SHIFT));
		coeff -= 4;
	}

	*ptr_delay_line = data;
  
  // For profiling only 
  cycle_num[1] = tile.cycles(); // cycle counter of the AI Engine tile
  printf("start = %d,end = %d,total = %d\n", cycle_num[0], cycle_num[1], cycle_num[1] - cycle_num[0]);
};

template <int SAMPLES, int SHIFT>
void Vector_32tap_fir_intrinsics<SAMPLES, SHIFT>::init() {
  for (int i = 0; i < 32; i++) { //initialize data
    int tmp = get_ss(0);
    delay_line[i] = *(cint16*)&tmp;
  }
};


/**
 * Compute 8/32 partial results for all samples in chunks of 16
 * 00	01 ... 06 07
 * 		10 11	... 16 17
 * 			 20 21 ... 26 27
 * 					30 31	... 36 37
 * 
 * 						 40 41 ... 46 47
 * 							  50 51 ... 56 57
 * 							     60 61 ... 66 67
 * 											70 71 ... 76 77
 * ...
 */
#define PARTIALMUL(XSTART, DOFFSET) \
	acc = mul4(data, XSTART, 0x3210, 1, taps, 0, 0x0000, 1); \
	acc = mac4(acc, data, XSTART+2, 0x3210, 1, taps, 2, 0x0000, 1); \
	acc = mac4(acc, data, XSTART+4, 0x3210, 1, taps, 4, 0x0000, 1); \
	data = upd_v(data, DOFFSET, readincr_v4(sin)); \
	acc = mac4(acc, data, XSTART+6, 0x3210, 1, taps, 6, 0x0000, 1);

template <int SAMPLES, int SHIFT>
void Multikernel_32tap_fir_intrinsics_core0<SAMPLES, SHIFT>::filter(
  input_stream<cint16>* sin,
  output_stream<cint16>* sout
) {
	v8cint16 taps = *(v8cint16*) weights;
	v16cint16 data = *(v16cint16*) delay_line;
	v4cacc48 acc = undef_v4cacc48();

	for (int i = 0; i < SAMPLES / 16; i++) {
		PARTIALMUL(0, 0);
		writeincr_v4(sout, srs(acc, SHIFT));
		PARTIALMUL(4, 1);
		writeincr_v4(sout, srs(acc, SHIFT));
		PARTIALMUL(8, 2);
		writeincr_v4(sout, srs(acc, SHIFT));
		PARTIALMUL(12, 3);
		writeincr_v4(sout, srs(acc, SHIFT));
	}
}

template <int SAMPLES, int SHIFT>
void Multikernel_32tap_fir_intrinsics_core0<SAMPLES, SHIFT>::init(const int delay) {
  for (int i = 0; i < delay; i++) get_ss(0);
	for (int i = 0; i < 16; i++) { //initialize data
    int tmp = get_ss(0);
    delay_line[i] = *(cint16*)&tmp;
  }
};