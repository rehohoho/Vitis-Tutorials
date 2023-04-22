#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <adf.h>
#include "mmul.h"

#define ITER_CNT  1
#define ROW_A_LEN 64
#define COL_A_LEN 64
#define COL_B_LEN 64
#define A_LEN     ROW_A_LEN * COL_A_LEN
#define B_LEN     COL_A_LEN * COL_B_LEN
#define C_LEN     ROW_A_LEN * COL_B_LEN


class MmulAieapiGraph : public adf::graph {

  private:
    adf::kernel k[1];

  public:
    adf::input_plio plin[2];
    adf::output_plio plout[1];

    MmulAieapiGraph() { 
      k[0] = adf::kernel::create_object<MmulAieapi<ROW_A_LEN, COL_A_LEN, COL_B_LEN, 0>>();
      adf::source(k[0]) = "mmul.cc";

#ifdef EXTERNAL_IO
      plin[0] = adf::input_plio::create("mmul_plin0", adf::plio_64_bits);
      plin[1] = adf::input_plio::create("mmul_plin1", adf::plio_64_bits);
      plout[0] = adf::output_plio::create("mmul_plout0", adf::plio_64_bits);
#else
      plin[0] = adf::input_plio::create("mmul_plin0", adf::plio_64_bits, "mmul_100samples.txt");
      plin[1] = adf::input_plio::create("mmul_plin1", adf::plio_64_bits, "mmul_100samples.txt");
      plout[0] = adf::output_plio::create("mmul_plout0", adf::plio_64_bits, "mmul_aieapi.txt");
#endif
      
      adf::connect<adf::window<A_LEN>> (plin[0].out[0], k[0].in[0]);
      adf::connect<adf::window<B_LEN>> (plin[1].out[0], k[0].in[1]);
      adf::connect<adf::window<C_LEN>> (k[0].out[0], plout[0].in[0]);
      adf::runtime<ratio>(k[0]) = 0.6;
    }

};

#endif // __GRAPH_H__