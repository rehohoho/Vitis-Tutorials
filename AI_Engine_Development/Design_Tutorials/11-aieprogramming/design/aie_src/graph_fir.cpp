#include "graph_fir.h"
#include "graph_utils.h"

// instance to be compiled and used in host within xclbin
// ScalarFirGraph sfir;
VectorFirGraph vfir;
// MultikernelFirGraph mfir;
// VectorIntrinsicFirGraph vifir;
MultikernelIntrinsicFirGraph mifir;

#ifdef __X86SIM__
int main(int argc, char ** argv) {
	// adfCheck(sfir.init(), "init sfir");
  // adfCheck(sfir.run(1), "run sfir");
	// adfCheck(sfir.end(), "end sfir");

  adfCheck(vfir.init(), "init vfir");
  adfCheck(vfir.run(1), "run vfir");
	adfCheck(vfir.end(), "end vfir");

  // adfCheck(mfir.init(), "init mfir");
  // adfCheck(mfir.run(1), "run mfir");
	// adfCheck(mfir.end(), "end mfir");
  
  // adfCheck(vifir.init(), "init vifir");
  // adfCheck(vifir.run(1), "run vifir");
	// adfCheck(vifir.end(), "end vifir");

  adfCheck(mifir.init(), "init mifir");
  adfCheck(mifir.run(1), "run mifir");
	adfCheck(mifir.end(), "end mifir");

  return 0;
}
#endif


#ifdef __AIESIM__
int main(int argc, char ** argv) {
	
	// adfCheck(sfir.init(), "init sfir");
  // get_graph_throughput_by_port(sfir, "plin1", sfir.plin1, SAMPLES, sizeof(cint16), ITER_CNT);
	// adfCheck(sfir.end(), "end sfir");

  adfCheck(vfir.init(), "init vfir");
  get_graph_throughput_by_port(vfir, "plin1", vfir.plin1, SAMPLES, sizeof(cint16), ITER_CNT);
	adfCheck(vfir.end(), "end vfir");

  // adfCheck(mfir.init(), "init mfir");
  // get_graph_throughput_by_port(mfir, "plin1", mfir.plin1, SAMPLES, sizeof(cint16), ITER_CNT);
	// adfCheck(mfir.end(), "end mfir");

  // adfCheck(vifir.init(), "init vifir");
  // get_graph_throughput_by_port(vifir, "plin1", vifir.plin1, SAMPLES, sizeof(cint16), ITER_CNT);
	// adfCheck(vifir.end(), "end vifir");
  
  adfCheck(mifir.init(), "init mifir");
  get_graph_throughput_by_port(mifir, "plin1", mifir.plin1, SAMPLES, sizeof(cint16), ITER_CNT);
	adfCheck(mifir.end(), "end mifir");
  return 0;
}
#endif
