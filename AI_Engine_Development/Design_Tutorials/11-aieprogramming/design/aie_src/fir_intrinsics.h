#ifndef FIR_INTRINSICS_H_
#define FIR_INTRINSICS_H_

#include <adf.h>
#include "aie_api/aie.hpp"


template <int SAMPLES, int SHIFT>
class Vector_32tap_fir_intrinsics {

  private:
    alignas(32) cint16 weights[32]; // ensure aligned for load and store
    alignas(32) cint16 delay_line[32]; // keep margin data between different executions of graph 
    void init(); // initialize data

  public:
    Vector_32tap_fir_intrinsics(const cint16 (&taps)[32]) {
      for (int i = 0; i < 32; i++) weights[i] = taps[i];
      init();
    };

    void filter(
      input_stream<cint16>* sin, 
      output_window<cint16>* __restrict sout);
    
    static void registerKernelClass() {
      REGISTER_FUNCTION(Vector_32tap_fir_intrinsics::filter);
    };

};


template <int SAMPLES, int SHIFT>
class Multikernel_32tap_fir_intrinsics_core0 {

  private:
    alignas(32) cint16 weights[8]; // ensure aligned for load and store
    alignas(32) cint16 delay_line[16]; // keep margin data between different executions of graph 
    void init(const int delay); // initialize data

  public:
    Multikernel_32tap_fir_intrinsics_core0(
      const cint16 (&taps)[8],
      const int delay
    ) {
      for (int i = 0; i < 8; i++) weights[i] = taps[i];
      init(delay);
    };

    void filter(
      input_stream<cint16>* sin, 
      output_stream<cacc48>* cout);
    
    static void registerKernelClass() {
      REGISTER_FUNCTION(Multikernel_32tap_fir_intrinsics_core0::filter);
    };

};


template <int SAMPLES, int SHIFT>
class Multikernel_32tap_fir_intrinsics_core1 {

  private:
    alignas(32) cint16 weights[8]; // ensure aligned for load and store
    alignas(32) cint16 delay_line[16]; // keep margin data between different executions of graph 
    void init(const int delay); // initialize data

  public:
    Multikernel_32tap_fir_intrinsics_core1(
      const cint16 (&taps)[8],
      const int delay
    ) {
      for (int i = 0; i < 8; i++) weights[i] = taps[i];
      init(delay);
    };

    void filter(
      input_stream<cint16>* sin, 
      input_stream<cacc48>* cin,
      output_stream<cacc48>* cout);
    
    static void registerKernelClass() {
      REGISTER_FUNCTION(Multikernel_32tap_fir_intrinsics_core1::filter);
    };

};


template <int SAMPLES, int SHIFT>
class Multikernel_32tap_fir_intrinsics_core3 {

  private:
    alignas(32) cint16 weights[8]; // ensure aligned for load and store
    alignas(32) cint16 delay_line[16]; // keep margin data between different executions of graph 
    void init(const int delay); // initialize data

  public:
    Multikernel_32tap_fir_intrinsics_core3(
      const cint16 (&taps)[8],
      const int delay
    ) {
      for (int i = 0; i < 8; i++) weights[i] = taps[i];
      init(delay);
    };

    void filter(
      input_stream<cint16>* sin, 
      input_stream<cacc48>* cin,
      output_window<cint16>* __restrict sout);
    
    static void registerKernelClass() {
      REGISTER_FUNCTION(Multikernel_32tap_fir_intrinsics_core3::filter);
    };

};


#endif // FIR_INTRINSICS_H_
