#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <adf.h>
#include "core01.h"
#include "core02.h"
#include "core03.h"
#include "core05.h"
#include "core04.h"

class LenetGraph : public adf::graph {

  private:
    adf::kernel core01;
    adf::kernel core02;
    adf::kernel core03;
    adf::kernel core05;
    adf::kernel core04;
    adf::parameter core01lut;
    adf::parameter core02lut;
    adf::parameter core03lut;
    adf::parameter core05lut;
    adf::parameter core04lut;

  public:
    adf::port<input> in[4];
    adf::port<output> out[3];

    LenetGraph() { 
      core01 = adf::kernel::create(core01_top);
      core02 = adf::kernel::create(core02_top);
      core03 = adf::kernel::create(core03_top);
      core05 = adf::kernel::create(core05_top);
      core04 = adf::kernel::create(core04_top);

      adf::location<adf::kernel>(core01) = adf::tile(8,0);
      adf::location<adf::kernel>(core02) = adf::tile(8,1);
      adf::location<adf::kernel>(core03) = adf::tile(8,2);
      adf::location<adf::kernel>(core05) = adf::tile(8,4);
      adf::location<adf::kernel>(core04) = adf::tile(8,3);

      core01lut = adf::parameter::array(B);
      core02lut = adf::parameter::array(B02);
      core03lut = adf::parameter::array(B03);
      core05lut = adf::parameter::array(B05);
      core04lut = adf::parameter::array(B04);

      adf::source(core01) = "core01.cc";
      adf::source(core02) = "core02.cc";
      adf::source(core03) = "core03.cc";
      adf::source(core05) = "core05.cc";
      adf::source(core04) = "core04.cc";

      adf::connect<>(core01lut,core01);
      adf::connect<>(core02lut,core02);
      adf::connect<>(core03lut,core03);
      adf::connect<>(core05lut,core05);
      adf::connect<>(core04lut,core04);

      adf::connect< adf::window<ROW_A * COL_A> > (in[0], core01.in[0]);
      adf::connect< adf::window<ROW_A * COL_B> > (core01.out[0], out[0]);

      adf::connect< adf::window<ROW_A_2 * COL_A_2> > (in[1], core02.in[0]);
      adf::connect< adf::window<ROW_A_2 * COL_B_2> > (core02.out[0], out[1]);

      adf::connect< adf::window<ROW_A_3 * COL_A_3> > (in[2], core03.in[0]);
      adf::connect< adf::window<ROW_A_5 * COL_A_5> > (in[3], core05.in[0]);
      adf::connect< adf::window<ROW_A_3 * COL_B_3> > (core03.out[0], core04.in[0]);
      adf::connect< adf::window<ROW_A_5 * COL_B_5> > (core05.out[0], core04.in[1]);
      adf::connect< adf::window<ROW_A_4 * COL_B_4> > (core04.out[0], out[2]);

      single_buffer(in[0]);
      single_buffer(in[1]);
      single_buffer(in[2]);
      single_buffer(in[3]);

      single_buffer(out[0]);
      single_buffer(out[1]);
      single_buffer(out[2]);

      single_buffer(core01.in[0]);
      single_buffer(core01.out[0]);

      single_buffer(core02.in[0]);
      single_buffer(core02.out[0]);

      single_buffer(core03.in[0]);
      single_buffer(core03.out[0]);

      single_buffer(core04.in[0]);
      single_buffer(core04.out[0]);

      single_buffer(core05.in[0]);
      single_buffer(core05.out[0]);
      //------
      adf::runtime<ratio>(core01) = 0.6;
      adf::runtime<ratio>(core02) = 0.6;
      adf::runtime<ratio>(core03) = 0.6;
      adf::runtime<ratio>(core05) = 0.6;
      adf::runtime<ratio>(core04) = 0.6;
    }
};

class SimGraph: public adf::graph {
  public:
    adf::input_plio plin1;
    adf::input_plio plin2;
    adf::input_plio plin3;
    adf::input_plio plin5;

    adf::output_plio plout1;
    adf::output_plio plout2;
    adf::output_plio plout4;

    LenetGraph lenet;

    SimGraph() {
#ifdef EXTERNAL_IO
      plin1 = adf::input_plio::create("plin1", adf::plio_64_bits);
      plin2 = adf::input_plio::create("plin2", adf::plio_64_bits);
      plin3 = adf::input_plio::create("plin3", adf::plio_64_bits);
      plin5 = adf::input_plio::create("plin5", adf::plio_64_bits);

      plout1 = adf::output_plio::create("plout1", adf::plio_64_bits);
      plout2 = adf::output_plio::create("plout2", adf::plio_64_bits);
      plout4 = adf::output_plio::create("plout4", adf::plio_64_bits);
#else
      plin1 = adf::input_plio::create("plin1", adf::plio_64_bits, "data/0_1/matA_in_64plio.txt");
      plin2 = adf::input_plio::create("plin2", adf::plio_64_bits, "data/0_2/matA_in_64plio.txt");
      plin3 = adf::input_plio::create("plin3", adf::plio_64_bits, "data/0_3/matA_in_64plio.txt");
      plin5 = adf::input_plio::create("plin5", adf::plio_64_bits, "data/0_5/matA_in_64plio.txt");

      plout1 = adf::output_plio::create("plout1", adf::plio_64_bits, "output01.txt");
      plout2 = adf::output_plio::create("plout2", adf::plio_64_bits, "output02.txt");
      plout4 = adf::output_plio::create("plout4", adf::plio_64_bits, "output04.txt");
#endif

      adf::connect<> (plin1.out[0], lenet.in[0]);
      adf::connect<> (plin2.out[0], lenet.in[1]);
      adf::connect<> (plin3.out[0], lenet.in[2]);
      adf::connect<> (plin5.out[0], lenet.in[3]);
      
      adf::connect<> (lenet.out[0], plout1.in[0]);
      adf::connect<> (lenet.out[1], plout2.in[0]);
      adf::connect<> (lenet.out[2], plout4.in[0]);
    }
};

#endif // __GRAPH_H__