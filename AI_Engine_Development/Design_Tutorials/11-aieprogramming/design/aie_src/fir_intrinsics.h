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
      for (int i = 0; i < 32; i++) {
        weights[i] = taps[i];
        // delay_line[i] = (cint16) {0, 0};
      }
      init();
    };

    void filter(
      input_stream<cint16>* sin, 
      output_stream<cint16>* sout);
    
    static void registerKernelClass() {
      REGISTER_FUNCTION(Vector_32tap_fir_intrinsics::filter);
    };

};

#endif // FIR_INTRINSICS_H_
