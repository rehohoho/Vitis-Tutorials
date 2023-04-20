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


template <int SAMPLES, int SHIFT>
class Multikernel_32tap_fir_core0 {

  private:
    alignas(aie::vector_decl_align) cint16 eq_coef[8];
    alignas(aie::vector_decl_align) aie::vector<cint16,16> delay_line; // store data between graph iterations

  public:
    Multikernel_32tap_fir_core0(
      const cint16 (&taps)[32], 
      const int delay
    ) {
      for (int i = 0 ; i < 8; i++) eq_coef[i] = taps[i];
      core0_init(delay);
    }

    __attribute__((noinline)) 
    void core0(
      input_stream<cint16>* sig_in, 
      output_stream<cacc48>* cascadeout);
    
    void core0_init(const int delay);

    static void registerKernelClass() {
      REGISTER_FUNCTION(Multikernel_32tap_fir_core0::core0);
    }
};


template <int SAMPLES, int SHIFT>
class Multikernel_32tap_fir_core1 {

  private:
    alignas(aie::vector_decl_align) cint16 eq_coef[8];
    alignas(aie::vector_decl_align) aie::vector<cint16,16> delay_line; // store data between graph iterations

  public:
    Multikernel_32tap_fir_core1(
      const cint16 (&taps)[32],
      const int delay
    ) {
      for (int i = 0 ; i < 8; i++) eq_coef[i] = taps[i+delay];
      core1_init(delay);
    }

    __attribute__((noinline)) 
    void core1(
      input_stream<cint16>* sig_in, 
      input_stream<cacc48>* cascadein,
      output_stream<cacc48>* cascadeout);
    
    void core1_init(const int delay);

    static void registerKernelClass() {
      REGISTER_FUNCTION(Multikernel_32tap_fir_core1::core1);
    }
};


template <int SAMPLES, int SHIFT>
class Multikernel_32tap_fir_core3 {

  private:
    alignas(aie::vector_decl_align) cint16 eq_coef[8];
    alignas(aie::vector_decl_align) aie::vector<cint16,16> delay_line; // store data between graph iterations

  public:
    Multikernel_32tap_fir_core3(
      const cint16 (&taps)[32],
      const int delay
    ) {
      for (int i = 0 ; i < 8; i++) eq_coef[i] = taps[i+delay];
      core3_init(delay);
    }

    __attribute__((noinline)) 
    void core3(
      input_stream<cint16>* sig_in, 
      input_stream<cacc48>* cascadein,
      output_stream<cint16>* data_out);
    
    void core3_init(const int delay);

    static void registerKernelClass() {
      REGISTER_FUNCTION(Multikernel_32tap_fir_core3::core3);
    }
};

#endif // FIR_H_
