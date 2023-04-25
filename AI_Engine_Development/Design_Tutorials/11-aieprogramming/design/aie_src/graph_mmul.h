#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <adf.h>
#include "mmul.h"
#include "mmul_intrinsics.h"

#define ITER_CNT  1
#define ROW_A_LEN 64
#define COL_A_LEN 64
#define COL_B_LEN 64
#define SHIFT_LEN 0
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
      k[0] = adf::kernel::create(mmul_aieapi_shuffleA<ROW_A_LEN, COL_A_LEN, COL_B_LEN, SHIFT_LEN>);
      k[1] = adf::kernel::create(mmul_aieapi_shuffleB<ROW_A_LEN, COL_A_LEN, COL_B_LEN, SHIFT_LEN>);
      k[2] = adf::kernel::create_object<MmulAieapi<ROW_A_LEN, COL_A_LEN, COL_B_LEN, SHIFT_LEN>>();
      k[3] = adf::kernel::create(mmul_aieapi_shuffleC<ROW_A_LEN, COL_A_LEN, COL_B_LEN, SHIFT_LEN>);
      
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


#define VMUL_M      64
#define VMUL_N      16
#define VMUL_A_LEN  VMUL_N * VMUL_M

class VmulScalarGraph : public adf::graph {

  private:
    adf::kernel k[1];

  public:
    adf::input_plio plin[2];
    adf::output_plio plout[1];

    VmulScalarGraph() { 
      k[0] = adf::kernel::create(vmul_intrinsic_scalar<VMUL_M, VMUL_N>);
      
      for (int i = 0; i < 1; i++)
        adf::source(k[i]) = "mmul_intrinsics.cc";

#ifdef EXTERNAL_IO
      plin[0] = adf::input_plio::create("vscalar_plin0", adf::plio_64_bits);
      plin[1] = adf::input_plio::create("vscalar_plin1", adf::plio_64_bits);
      plout[0] = adf::output_plio::create("vscalar_plout0", adf::plio_64_bits);
#else
      plin[0] = adf::input_plio::create("vscalar_plin0", adf::plio_64_bits, "vmula_100samples.txt");
      plin[1] = adf::input_plio::create("vscalar_plin1", adf::plio_64_bits, "vmulb_100samples.txt");
      plout[0] = adf::output_plio::create("vscalar_plout0", adf::plio_64_bits, "vmul_scalar.txt");
#endif
      
      adf::connect<adf::window<VMUL_A_LEN*2>> (plin[0].out[0], k[0].in[0]);
      adf::connect<adf::window<VMUL_N*2>> (plin[1].out[0], k[0].in[1]);
      adf::connect<adf::window<VMUL_M*2>> (k[0].out[0], plout[0].in[0]);
      
      for (int i = 0; i < 1; i++)
        adf::runtime<ratio>(k[i]) = 0.6;
    }

};


class VmulVectorGraph : public adf::graph {

  private:
    adf::kernel k[1];

  public:
    adf::input_plio plin[2];
    adf::output_plio plout[1];

    VmulVectorGraph() { 
      k[0] = adf::kernel::create(vmul_intrinsic_vector<VMUL_M, VMUL_N>);
      
      for (int i = 0; i < 1; i++)
        adf::source(k[i]) = "mmul_intrinsics.cc";

#ifdef EXTERNAL_IO
      plin[0] = adf::input_plio::create("vvector_plin0", adf::plio_64_bits);
      plin[1] = adf::input_plio::create("vvector_plin1", adf::plio_64_bits);
      plout[0] = adf::output_plio::create("vvector_plout0", adf::plio_64_bits);
#else
      plin[0] = adf::input_plio::create("vvector_plin0", adf::plio_64_bits, "vmula_100samples.txt");
      plin[1] = adf::input_plio::create("vvector_plin1", adf::plio_64_bits, "vmulb_100samples.txt");
      plout[0] = adf::output_plio::create("vvector_plout0", adf::plio_64_bits, "vmul_vector.txt");
#endif
      
      adf::connect<adf::window<VMUL_A_LEN*2>> (plin[0].out[0], k[0].in[0]);
      adf::connect<adf::window<VMUL_N*2>> (plin[1].out[0], k[0].in[1]);
      adf::connect<adf::window<VMUL_M*2>> (k[0].out[0], plout[0].in[0]);
      
      for (int i = 0; i < 1; i++)
        adf::runtime<ratio>(k[i]) = 0.6;
    }

};


class Vmul2VectorGraph : public adf::graph {

  private:
    adf::kernel k[1];

  public:
    adf::input_plio plin[3];
    adf::output_plio plout[1];

    Vmul2VectorGraph() { 
      k[0] = adf::kernel::create(vmul_intrinsic_vector_2matA<VMUL_M, VMUL_N>);
      
      for (int i = 0; i < 1; i++)
        adf::source(k[i]) = "mmul_intrinsics.cc";

#ifdef EXTERNAL_IO
      plin[0] = adf::input_plio::create("v2vector_plin0", adf::plio_64_bits);
      plin[1] = adf::input_plio::create("v2vector_plin1", adf::plio_64_bits);
      plin[2] = adf::input_plio::create("v2vector_plin2", adf::plio_64_bits);
      plout[0] = adf::output_plio::create("v2vector_plout0", adf::plio_64_bits);
#else
      plin[0] = adf::input_plio::create("v2vector_plin0", adf::plio_64_bits, "vmula_100samples_phase1.txt");
      plin[1] = adf::input_plio::create("v2vector_plin1", adf::plio_64_bits, "vmula_100samples_phase2.txt");
      plin[2] = adf::input_plio::create("v2vector_plin2", adf::plio_64_bits, "vmulb_100samples.txt");
      plout[0] = adf::output_plio::create("v2vector_plout0", adf::plio_64_bits, "vmul_2vector.txt");
#endif
      
      adf::connect<adf::window<VMUL_A_LEN*2/2>> (plin[0].out[0], k[0].in[0]);
      adf::connect<adf::window<VMUL_A_LEN*2/2>> (plin[1].out[0], k[0].in[1]);
      adf::connect<adf::window<VMUL_N*2>> (plin[2].out[0], k[0].in[2]);
      adf::connect<adf::window<VMUL_M*2>> (k[0].out[0], plout[0].in[0]);
      
      for (int i = 0; i < 1; i++)
        adf::runtime<ratio>(k[i]) = 0.6;
    }

};


#define MMUL_M      64
#define MMUL_K      8
#define MMUL_N      2
#define MMUL_A_LEN  MMUL_M * MMUL_K
#define MMUL_B_LEN  MMUL_K * MMUL_N
#define MMUL_C_LEN  MMUL_M * MMUL_N

class MmulScalarGraph : public adf::graph {

  private:
    adf::kernel k[1];

  public:
    adf::input_plio plin[2];
    adf::output_plio plout[1];

    MmulScalarGraph() { 
      k[0] = adf::kernel::create(mmul_intrinsic_scalar<MMUL_M, MMUL_K, MMUL_N>);
      
      for (int i = 0; i < 1; i++)
        adf::source(k[i]) = "mmul_intrinsics.cc";

#ifdef EXTERNAL_IO
      plin[0] = adf::input_plio::create("mscalar_plin0", adf::plio_64_bits);
      plin[1] = adf::input_plio::create("mscalar_plin1", adf::plio_64_bits);
      plout[0] = adf::output_plio::create("mscalar_plout0", adf::plio_64_bits);
#else
      plin[0] = adf::input_plio::create("mscalar_plin0", adf::plio_64_bits, "mmula_100samples.txt");
      plin[1] = adf::input_plio::create("mscalar_plin1", adf::plio_64_bits, "mmulb_100samples.txt");
      plout[0] = adf::output_plio::create("mscalar_plout0", adf::plio_64_bits, "mmul_scalar.txt");
#endif
      
      adf::connect<adf::window<MMUL_A_LEN*2>> (plin[0].out[0], k[0].in[0]);
      adf::connect<adf::window<MMUL_B_LEN*2>> (plin[1].out[0], k[0].in[1]);
      adf::connect<adf::window<MMUL_C_LEN*2>> (k[0].out[0], plout[0].in[0]);
      
      for (int i = 0; i < 1; i++)
        adf::runtime<ratio>(k[i]) = 0.6;
    }

};


class MmulVectorGraph : public adf::graph {

  private:
    adf::kernel k[1];

  public:
    adf::input_plio plin[2];
    adf::output_plio plout[1];

    MmulVectorGraph() { 
      k[0] = adf::kernel::create(mmul_intrinsic_vector<MMUL_M, MMUL_K, MMUL_N>);
      
      for (int i = 0; i < 1; i++)
        adf::source(k[i]) = "mmul_intrinsics.cc";

#ifdef EXTERNAL_IO
      plin[0] = adf::input_plio::create("mvector_plin0", adf::plio_64_bits);
      plin[1] = adf::input_plio::create("mvector_plin1", adf::plio_64_bits);
      plout[0] = adf::output_plio::create("mvector_plout0", adf::plio_64_bits);
#else
      plin[0] = adf::input_plio::create("mvector_plin0", adf::plio_64_bits, "mmula_100samples.txt");
      plin[1] = adf::input_plio::create("mvector_plin1", adf::plio_64_bits, "mmulb_100samples.txt");
      plout[0] = adf::output_plio::create("mvector_plout0", adf::plio_64_bits, "mmul_vector.txt");
#endif
      
      adf::connect<adf::window<MMUL_A_LEN*2>> (plin[0].out[0], k[0].in[0]);
      adf::connect<adf::window<MMUL_B_LEN*2>> (plin[1].out[0], k[0].in[1]);
      adf::connect<adf::window<MMUL_C_LEN*2>> (k[0].out[0], plout[0].in[0]);
      
      for (int i = 0; i < 1; i++)
        adf::runtime<ratio>(k[i]) = 0.6;
    }

};


#endif // __GRAPH_H__