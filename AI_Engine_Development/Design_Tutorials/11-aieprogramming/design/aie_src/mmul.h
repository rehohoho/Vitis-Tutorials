#ifndef MMUL_H_
#define MMUL_H_

#include <adf.h>
#include "aie_api/aie.hpp"

#define M 4
#define K 16
#define N 8


template <int ROWA, int COLA, int COLB, int SHIFT>
class MmulAieapi {

  private:
		const int num_rowA = ROWA/M; 
		const int num_colA = COLA/K;
		const int num_colB = COLB/N;

  public:
		MmulAieapi() {};

    void filter(
			input_window<int8> * __restrict matA, 
			input_window<int8> * __restrict matB, 
			output_window<int8> * __restrict matC);
    
    static void registerKernelClass() {
      REGISTER_FUNCTION(MmulAieapi::filter);
    };

};

#endif // MMUL_H_