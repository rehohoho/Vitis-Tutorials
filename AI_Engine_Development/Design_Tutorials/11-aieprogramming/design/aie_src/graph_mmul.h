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
    adf::kernel k[4];

  public:
    adf::input_plio plin[2];
    adf::output_plio plout[1];

    MmulAieapiGraph() { 
      k[0] = adf::kernel::create(mmul_aieapi_shuffleA<ROW_A_LEN, COL_A_LEN, COL_B_LEN, 0>);
      k[1] = adf::kernel::create(mmul_aieapi_shuffleB<ROW_A_LEN, COL_A_LEN, COL_B_LEN, 0>);
      k[2] = adf::kernel::create_object<MmulAieapi<ROW_A_LEN, COL_A_LEN, COL_B_LEN, 0>>();
      k[3] = adf::kernel::create(mmul_aieapi_shuffleC<ROW_A_LEN, COL_A_LEN, COL_B_LEN, 0>);
      
      for (int i = 0; i < 4; i++)
        adf::source(k[i]) = "mmul.cc";

#ifdef EXTERNAL_IO
      plin[0] = adf::input_plio::create("mmul_plin0", adf::plio_64_bits);
      plin[1] = adf::input_plio::create("mmul_plin1", adf::plio_64_bits);
      plout[0] = adf::output_plio::create("mmul_plout0", adf::plio_64_bits);
#else
      plin[0] = adf::input_plio::create("mmul_plin0", adf::plio_64_bits, "mmul_100samples_rand.txt");
      plin[1] = adf::input_plio::create("mmul_plin1", adf::plio_64_bits, "mmul_100samples_rand.txt");
      plout[0] = adf::output_plio::create("mmul_plout0", adf::plio_64_bits, "mmul_aieapi.txt");
#endif
      
      adf::connect<adf::window<A_LEN>> (plin[0].out[0], k[0].in[0]);
      adf::connect<adf::window<B_LEN>> (plin[1].out[0], k[1].in[0]);
      
      adf::connect<adf::window<A_LEN>> (k[0].out[0], k[2].in[0]);
      adf::connect<adf::window<B_LEN>> (k[1].out[0], k[2].in[1]);
      adf::connect<adf::window<C_LEN>> (k[2].out[0], k[3].in[0]);
      
      adf::connect<adf::window<C_LEN>> (k[3].out[0], plout[0].in[0]);
      
      for (int i = 0; i < 4; i++)
        adf::runtime<ratio>(k[i]) = 0.6;
    }

};

#endif // __GRAPH_H__