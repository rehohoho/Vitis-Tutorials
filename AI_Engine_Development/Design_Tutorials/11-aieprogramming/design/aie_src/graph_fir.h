#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <adf.h>
#include "fir.h"
#include "fir_intrinsics.h"

#define SAMPLES  64  // use shorter for faster runtime
#define SHIFT    0   // to analyze output generated by impulses at input, use 15 for realistic input
#define ITER_CNT 1

std::vector<cint16> taps = std::vector<cint16>({
  {0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7}, 
  {0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7}, 
  {0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7}, 
  {0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7}
});


class ScalarFirGraph : public adf::graph {

  private:
    adf::kernel fir;

  public:
    adf::input_plio plin1;
    adf::output_plio plout1;

    ScalarFirGraph() { 
      fir = adf::kernel::create_object<Scalar_32tap_fir<SAMPLES, SHIFT>>(taps);
      adf::source(fir) = "fir.cc";

#ifdef EXTERNAL_IO
      plin1 = adf::input_plio::create("sfir_plin1", adf::plio_64_bits);
      plout1 = adf::output_plio::create("sfir_plout1", adf::plio_64_bits);
#else
      plin1 = adf::input_plio::create("sfir_plin1", adf::plio_64_bits, "fir_10samples.txt");
      plout1 = adf::output_plio::create("sfir_plout1", adf::plio_64_bits, "scalar_fir.txt");
#endif
      
      adf::connect<adf::stream> n0 (plin1.out[0], fir.in[0]);
      adf::connect<adf::stream> n1 (fir.out[0], plout1.in[0]);
      
      adf::runtime<ratio>(fir) = 0.6;
    }

};


class VectorFirGraph : public adf::graph {

  private:
    adf::kernel fir;

  public:
    adf::input_plio plin1;
    adf::output_plio plout1;

    VectorFirGraph() { 
      fir = adf::kernel::create_object<Vector_32tap_fir<SAMPLES, SHIFT>>(taps);
      adf::source(fir) = "fir.cc";

#ifdef EXTERNAL_IO
      plin1 = adf::input_plio::create("vfir_plin1", adf::plio_64_bits);
      plout1 = adf::output_plio::create("vfir_plout1", adf::plio_64_bits);
#else
      plin1 = adf::input_plio::create("vfir_plin1", adf::plio_64_bits, "fir_10samples.txt");
      plout1 = adf::output_plio::create("vfir_plout1", adf::plio_64_bits, "vector_fir.txt");
#endif
      
      adf::connect<adf::stream> n0 (plin1.out[0], fir.in[0]);
      adf::connect<adf::stream> n1 (fir.out[0], plout1.in[0]);
      
      adf::runtime<ratio>(fir) = 0.6;
    }

};


class MultikernelFirGraph : public adf::graph {

  private:
    adf::kernel k[4];

  public:
    adf::input_plio plin1;
    adf::output_plio plout1;

    MultikernelFirGraph() { 
      k[0] = adf::kernel::create_object<Multikernel_32tap_fir_core0<SAMPLES, SHIFT>>(taps, 0);
      k[1] = adf::kernel::create_object<Multikernel_32tap_fir_core1<SAMPLES, SHIFT>>(taps, 8);
      k[2] = adf::kernel::create_object<Multikernel_32tap_fir_core1<SAMPLES, SHIFT>>(taps, 16);
      k[3] = adf::kernel::create_object<Multikernel_32tap_fir_core3<SAMPLES, SHIFT>>(taps, 24);
      
      for (int i = 0; i < 4; i++) 
        adf::source(k[i]) = "fir.cc";

#ifdef EXTERNAL_IO
      plin1 = adf::input_plio::create("mfir_plin1", adf::plio_64_bits);
      plout1 = adf::output_plio::create("mfir_plout1", adf::plio_64_bits);
#else
      plin1 = adf::input_plio::create("mfir_plin1", adf::plio_64_bits, "fir_10samples.txt");
      plout1 = adf::output_plio::create("mfir_plout1", adf::plio_64_bits, "multikernel_fir.txt");
#endif
      
      for (int i = 0; i < 4; i++) 
        adf::connect<adf::stream> (plin1.out[0], k[i].in[0]);      
      for (int i = 0 ; i < 3; i++)
        adf::connect<adf::cascade> (k[i].out[0], k[i+1].in[1]);
      adf::connect<adf::stream> (k[3].out[0], plout1.in[0]);
      
      for (int i = 0; i < 4; i++) 
        adf::runtime<ratio>(k[i]) = 0.6;
    }

};


class VectorIntrinsicFirGraph : public adf::graph {

  private:
    adf::kernel fir;

  public:
    adf::input_plio plin1;
    adf::output_plio plout1;

    VectorIntrinsicFirGraph() { 
      fir = adf::kernel::create_object<Vector_32tap_fir_intrinsics<SAMPLES, SHIFT>>(taps);
      adf::source(fir) = "fir_intrinsics.cc";

#ifdef EXTERNAL_IO
      plin1 = adf::input_plio::create("vifir_plin1", adf::plio_64_bits);
      plout1 = adf::output_plio::create("vifir_plout1", adf::plio_64_bits);
#else
      plin1 = adf::input_plio::create("vifir_plin1", adf::plio_64_bits, "fir_10samples.txt");
      plout1 = adf::output_plio::create("vifir_plout1", adf::plio_64_bits, "vector_intrinsics_fir.txt");
#endif
      
      adf::connect<adf::stream> n0 (plin1.out[0], fir.in[0]);
      adf::connect<adf::stream> n1 (fir.out[0], plout1.in[0]);
      
      adf::runtime<ratio>(fir) = 0.6;
    }

};

#define GET_PHASE(START,STEP) {\
	taps[START],taps[START+STEP],taps[START+2*STEP],taps[START+3*STEP],\
	taps[START+4*STEP],taps[START+5*STEP],taps[START+6*STEP],taps[START+7*STEP]}
std::vector<cint16> taps4_p0 = std::vector<cint16>(GET_PHASE(0,1));
std::vector<cint16> taps4_p1 = std::vector<cint16>(GET_PHASE(8,1));
std::vector<cint16> taps4_p2 = std::vector<cint16>(GET_PHASE(16,1));
std::vector<cint16> taps4_p3 = std::vector<cint16>(GET_PHASE(24,1));

class MultikernelIntrinsicFirGraph : public adf::graph {

  private:
    adf::kernel k[4];

  public:
    adf::input_plio plin1;
    adf::output_plio plout1;

    MultikernelIntrinsicFirGraph() { 
      k[0] = adf::kernel::create_object<Multikernel_32tap_fir_intrinsics_core0<SAMPLES, SHIFT>>(taps4_p0, 0);
      k[1] = adf::kernel::create_object<Multikernel_32tap_fir_intrinsics_core1<SAMPLES, SHIFT>>(taps4_p1, 8);
      k[2] = adf::kernel::create_object<Multikernel_32tap_fir_intrinsics_core1<SAMPLES, SHIFT>>(taps4_p2, 16);
      k[3] = adf::kernel::create_object<Multikernel_32tap_fir_intrinsics_core3<SAMPLES, SHIFT>>(taps4_p3, 24);
      
      for (int i = 0; i < 4; i++)
        adf::source(k[i]) = "fir_intrinsics.cc";

#ifdef EXTERNAL_IO
      plin1 = adf::input_plio::create("mifir_plin1", adf::plio_64_bits);
      plout1 = adf::output_plio::create("mifir_plout1", adf::plio_64_bits);
#else
      plin1 = adf::input_plio::create("mifir_plin1", adf::plio_64_bits, "fir_10samples.txt");
      plout1 = adf::output_plio::create("mifir_plout1", adf::plio_64_bits, "multikernel_intrinsics_fir.txt");
#endif
      
      for (int i = 0; i < 4; i++)
        adf::connect<adf::stream> (plin1.out[0], k[i].in[0]);
      for (int i = 0; i < 3; i++)
        adf::connect<adf::cascade> (k[i].out[0], k[i+1].in[1]);
      adf::connect<adf::stream> (k[3].out[0], plout1.in[0]);
      
      for (int i = 0; i < 4; i++)
        adf::runtime<ratio>(k[i]) = 0.6;
    }

};

#endif // __GRAPH_H__