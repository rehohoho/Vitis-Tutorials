#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <adf.h>
#include "mul.h"


class ScalarMulGraph : public adf::graph {

  private:
    adf::kernel mul;

  public:
    adf::input_plio plin1;
    adf::input_plio plin2;
    adf::output_plio plout1;

    ScalarMulGraph() { 
      mul = adf::kernel::create(scalar_mul);
      adf::source(mul) = "mul.cc";

#ifdef EXTERNAL_IO
      plin1 = adf::input_plio::create("smul_plin1", adf::plio_64_bits);
      plin2 = adf::input_plio::create("smul_plin2", adf::plio_64_bits);
      plout1 = adf::output_plio::create("smul_plout1", adf::plio_64_bits);
#else
      plin1 = adf::input_plio::create("smul_plin1", adf::plio_64_bits, "va_10samples.txt");
      plin2 = adf::input_plio::create("smul_plin2", adf::plio_64_bits, "vb_10samples.txt");
      plout1 = adf::output_plio::create("smul_plout1", adf::plio_64_bits, "scalar_mul.txt");
#endif

      adf::connect<adf::window<V_LEN*4>>(plin1.out[0], mul.in[0]);
      adf::connect<adf::window<V_LEN*4>>(plin2.out[0], mul.in[1]);
      adf::connect<adf::window<V_LEN*4>>(mul.out[0], plout1.in[0]);
      
      adf::runtime<ratio>(mul) = 0.6;
    }

};


class VectorMulGraph : public adf::graph {

  private:
    adf::kernel mul;

  public:
    adf::input_plio plin1;
    adf::input_plio plin2;
    adf::output_plio plout1;

    VectorMulGraph() { 
      mul = adf::kernel::create(vector_mul);
      adf::source(mul) = "mul.cc";

#ifdef EXTERNAL_IO
      plin1 = adf::input_plio::create("vmul_plin1", adf::plio_64_bits);
      plin2 = adf::input_plio::create("vmul_plin2", adf::plio_64_bits);
      plout1 = adf::output_plio::create("vmul_plout1", adf::plio_64_bits);
#else
      plin1 = adf::input_plio::create("vmul_plin1", adf::plio_64_bits, "va_10samples.txt");
      plin2 = adf::input_plio::create("vmul_plin2", adf::plio_64_bits, "vb_10samples.txt");
      plout1 = adf::output_plio::create("vmul_plout1", adf::plio_64_bits, "vector_mul.txt");
#endif

      adf::connect<adf::window<V_LEN*4>>(plin1.out[0], mul.in[0]);
      adf::connect<adf::window<V_LEN*4>>(plin2.out[0], mul.in[1]);
      adf::connect<adf::window<V_LEN*4>>(mul.out[0], plout1.in[0]);
      
      adf::runtime<ratio>(mul) = 0.6;
    }

};

#endif // __GRAPH_H__