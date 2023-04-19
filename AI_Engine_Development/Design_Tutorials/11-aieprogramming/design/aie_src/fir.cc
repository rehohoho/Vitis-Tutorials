#include <stdlib.h>
#include <stdio.h>
#include "aie_api/aie.hpp"

#include "fir.h"


template <int SAMPLES, int SHIFT>
__attribute__((noinline)) 
void Scalar_32tap_fir<SAMPLES, SHIFT>::filter(
  input_stream<cint16>* sig_in,
  output_stream<cint16>* sig_out
) {
  // For profiling only 
  unsigned cycle_num[2];
  aie::tile tile = aie::tile::current();
  cycle_num[0] = tile.cycles(); // cycle counter of the AI Engine tile

  for(int i = 0; i < SAMPLES; i++){
    cint64 sum = {0,0};// larger data to mimic accumulator
    for(int j = 0; j < 32; j++){
      // auto integer promotion to prevent overflow
      sum.real += delay_line[j].real * eq_coef[j].real - delay_line[j].imag * eq_coef[j].imag;
      sum.imag += delay_line[j].real * eq_coef[j].imag + delay_line[j].imag * eq_coef[j].real;
    }
    sum = sum >> SHIFT;
    
    // produce one sample per loop iteration
    writeincr(sig_out, {(int16) sum.real, (int16) sum.imag});

    // shift window
    for(int j = 0; j < 32; j++){
      if (j == 31) {
        delay_line[j] = readincr(sig_in);
      }else{
        delay_line[j] = delay_line[j+1];
      }
    }
  }
  
  // For profiling only 
  cycle_num[1] = tile.cycles(); // cycle counter of the AI Engine tile
  printf("start = %d,end = %d,total = %d\n", cycle_num[0], cycle_num[1], cycle_num[1] - cycle_num[0]);
}

// initialize data
template <int SAMPLES, int SHIFT>
void Scalar_32tap_fir<SAMPLES, SHIFT>::scalar_32tap_fir_init() {
  for (int i = 0; i < 32; i++){
    int tmp = get_ss(0);
    delay_line[i] = *(cint16*)&tmp;
  }
};