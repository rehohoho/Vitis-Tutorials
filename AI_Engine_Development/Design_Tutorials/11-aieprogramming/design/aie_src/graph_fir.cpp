#include "graph_fir.h"
#include "graph_utils.h"

// instance to be compiled and used in host within xclbin
ScalarFirGraph sfir;
VectorFirGraph vfir;


#ifdef __X86SIM__
int main(int argc, char ** argv) {
	adfCheck(sfir.init(), "init sfir");
  adfCheck(sfir.run(1), "run sfir");
	adfCheck(sfir.end(), "end sfir");

  adfCheck(vfir.init(), "init vfir");
  adfCheck(vfir.run(1), "run vfir");
	adfCheck(vfir.end(), "end vfir");
  return 0;
}
#endif


#ifdef __AIESIM__
int main(int argc, char ** argv) {
	
	adfCheck(sfir.init(), "init sfir");
  get_graph_throughput_by_port(sfir, "plin1", sfir.plin1, SAMPLES, sizeof(cint16), ITER_CNT);
	adfCheck(sfir.end(), "end sfir");

  adfCheck(vfir.init(), "init vfir");
  get_graph_throughput_by_port(vfir, "plin1", vfir.plin1, SAMPLES, sizeof(cint16), ITER_CNT);
	adfCheck(vfir.end(), "end vfir");
  
  return 0;
}
#endif
