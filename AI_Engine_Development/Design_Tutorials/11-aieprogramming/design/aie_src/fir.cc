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
    cint64 sum = {0, 0};// larger data to mimic accumulator
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
void Scalar_32tap_fir<SAMPLES, SHIFT>::init() {
  for (int i = 0; i < 32; i++){
    int tmp = get_ss(0);
    delay_line[i] = *(cint16*)&tmp;
  }
};


template <int SAMPLES, int SHIFT>
__attribute__((noinline)) 
void Vector_32tap_fir<SAMPLES, SHIFT>::filter(
  input_stream<cint16>* sig_in,
  output_stream<cint16>* sig_out
) {
  // For profiling only 
  unsigned cycle_num[2];
  aie::tile tile = aie::tile::current();
  cycle_num[0] = tile.cycles(); // cycle counter of the AI Engine tile

  const aie::vector<cint16, 8> coe[4] = {
    aie::load_v<8>(eq_coef), 
    aie::load_v<8>(eq_coef+8), 
    aie::load_v<8>(eq_coef+16), 
    aie::load_v<8>(eq_coef+24)
  };
  aie::vector<cint16, 32> buff = delay_line;
  aie::accum<cacc48, 8> acc;
  
  for (int i = 0; i < SAMPLES / 32; i++) {
    //performace 1st 8 samples
    acc = aie::sliding_mul<8, 8>(coe[0], 0, buff, 0);
    acc = aie::sliding_mac<8, 8>(acc, coe[1], 0, buff, 8);
    buff.insert(0, readincr_v<4>(sig_in)); // first 8 with new data
    buff.insert(1, readincr_v<4>(sig_in)); // interleaving loads allow load+MAC in same cycle
    acc = aie::sliding_mac<8, 8>(acc, coe[2], 0, buff, 16);
    acc = aie::sliding_mac<8, 8>(acc, coe[3], 0, buff, 24);
    writeincr(sig_out, acc.to_vector<cint16>(SHIFT));

    //performace 2nd 8 samples
    acc = aie::sliding_mul<8, 8>(coe[0], 0, buff, 8);
    acc = aie::sliding_mac<8, 8>(acc, coe[1], 0, buff, 16);
    buff.insert(2, readincr_v<4>(sig_in));  // update next 8 with new data
    buff.insert(3, readincr_v<4>(sig_in));
    acc = aie::sliding_mac<8, 8>(acc, coe[2], 0, buff, 24);
    acc = aie::sliding_mac<8, 8>(acc, coe[3], 0, buff, 0); // uses first 8 new data
    writeincr(sig_out, acc.to_vector<cint16>(SHIFT));

    //performace 3rd 8 samples
    acc = aie::sliding_mul<8, 8>(coe[0], 0, buff, 16);
    acc = aie::sliding_mac<8, 8>(acc, coe[1], 0, buff, 24);
    buff.insert(4, readincr_v<4>(sig_in));
    buff.insert(5, readincr_v<4>(sig_in));
    acc = aie::sliding_mac<8, 8>(acc, coe[2], 0, buff, 0);
    acc = aie::sliding_mac<8, 8>(acc, coe[3], 0, buff, 8);
    writeincr(sig_out, acc.to_vector<cint16>(SHIFT));

    //performace 4th 8 samples
    acc = aie::sliding_mul<8, 8>(coe[0], 0, buff, 24);
    acc = aie::sliding_mac<8, 8>(acc, coe[1], 0, buff, 0);
    buff.insert(6, readincr_v<4>(sig_in)); // update last 8 with new data
    buff.insert(7, readincr_v<4>(sig_in));
    acc = aie::sliding_mac<8, 8>(acc, coe[2], 0, buff, 8);
    acc = aie::sliding_mac<8, 8>(acc, coe[3], 0, buff, 16);
    writeincr(sig_out, acc.to_vector<cint16>(SHIFT));
  }

  delay_line = buff;

  // For profiling only 
  cycle_num[1] = tile.cycles(); // cycle counter of the AI Engine tile
  printf("start = %d,end = %d,total = %d\n", cycle_num[0], cycle_num[1], cycle_num[1] - cycle_num[0]);
};


template <int SAMPLES, int SHIFT>
void Vector_32tap_fir<SAMPLES, SHIFT>::init() {
  //initialize data
  for (int i = 0; i < 8; i++){
    aie::vector<int16, 8> tmp = get_wss(0);
    delay_line.insert(i, tmp.cast_to<cint16>());
  }
};


// aicompiler.log: 16 cycles for 16 partial results, 1 cycle / partial result
template <int SAMPLES, int SHIFT>
__attribute__((noinline)) // optional: keep function hierarchy
void Multikernel_32tap_fir<SAMPLES, SHIFT>::core0(
  input_stream<cint16>* sig_in,
  output_stream<cint16>* sig_out
){
  const cint16_t * restrict coeff = eq_coef0;
  const aie::vector<cint16, 8> coe = aie::load_v<8>(coeff);

  aie::vector<cint16, 16> buff = delay_line;
  aie::accum<cacc48, 4> acc;
  const unsigned LSIZE = (SAMPLES/4/4); // assuming samples is integer power of 2 and greater than 16

  for (unsigned int i = 0; i < LSIZE; ++i) chess_prepare_for_pipelining { // optional: tool can do autopipelining 
    buff.insert(2, readincr_v<4>(sig_in));
    acc = aie::sliding_mul<4, 8>(coe, 0, buff, 0); // 8 MAC produce 4 partial output
    writeincr_v4(sig_out, acc.to_vector<cint16>(SHIFT));

    buff.insert(3, readincr_v<4>(sig_in));
    acc = aie::sliding_mul<4, 8>(coe, 0, buff, 4);
    writeincr_v4(sig_out, acc.to_vector<cint16>(SHIFT));

    buff.insert(0, readincr_v<4>(sig_in));
    acc = aie::sliding_mul<4, 8>(coe, 0, buff, 8);
    writeincr_v4(sig_out, acc.to_vector<cint16>(SHIFT));

    buff.insert(1, readincr_v<4>(sig_in));
    acc = aie::sliding_mul<4, 8>(coe, 0, buff, 12);
    writeincr_v4(sig_out, acc.to_vector<cint16>(SHIFT));
  }
  delay_line = buff;
}


template <int SAMPLES, int SHIFT>
void Multikernel_32tap_fir<SAMPLES, SHIFT>::core0_init() {
  int const Delay = 0;
  for (int i = 0; i < Delay; ++i) get_ss(0); // Drop samples if not first block
  for (int i = 0; i < 8; i++) { // Initialize data
    int tmp = get_ss(0);
    delay_line.set(*(cint16*)&tmp, i);
  }
};
