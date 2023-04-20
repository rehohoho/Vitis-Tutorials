#ifndef FIR_H_
#define FIR_H_

#include <adf.h>
#include "aie_api/aie.hpp"


template <int SAMPLES, int SHIFT>
class Scalar_32tap_fir {

  private:
    alignas(32) cint16 eq_coef[32];
    alignas(32) cint16 delay_line[32]; // keep margin data between different executions of graph 
    void init(); // initialize data

  public:
    Scalar_32tap_fir(const cint16 (&taps)[32]) {
      for (int i = 0; i < 32; i++) eq_coef[i] = taps[i];
      init();
    }

    __attribute__((noinline)) 
    void filter(
      input_stream<cint16>* sig_in, 
      output_stream<cint16>* sig_out);
    
    static void registerKernelClass() {
      REGISTER_FUNCTION(Scalar_32tap_fir::filter);
    }

};


template <int SAMPLES, int SHIFT>
class Vector_32tap_fir {

  private:
    alignas(aie::vector_decl_align) cint16 eq_coef[32]; // ensure aligned for load and store
    alignas(32) aie::vector<cint16,32> delay_line; // keep margin data between different executions of graph 
    void init(); // initialize data

  public:
    Vector_32tap_fir(const cint16 (&taps)[32]) {
      for (int i = 0; i < 32; i++) eq_coef[i] = taps[i];
      init();
    }

    __attribute__((noinline)) 
    void filter(
      input_stream<cint16>* sig_in, 
      output_stream<cint16>* sig_out);
    
    static void registerKernelClass() {
      REGISTER_FUNCTION(Vector_32tap_fir::filter);
    }

};

#endif // FIR_H_
