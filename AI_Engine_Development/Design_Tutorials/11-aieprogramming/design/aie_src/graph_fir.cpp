#include "graph_fir.h"
#include "graph_utils.h"

// instance to be compiled and used in host within xclbin
ScalarFirGraph sfir;


#ifdef __X86SIM__
int main(int argc, char ** argv) {
	adfCheck(sfir.init(), "init sfir");
  adfCheck(sfir.run(1), "run sfir");
	adfCheck(sfir.end(), "end sfir");
  return 0;
}
#endif


#ifdef __AIESIM__
int main(int argc, char ** argv) {
	
	adfCheck(sfir.init(), "init sfir");
  get_graph_latency(sfir, "plin1/plout1", sfir.plin1, sfir.plout1, ITER_CNT);
  get_graph_throughput_by_port(sfir, "plin1", sfir.plin1, V_LEN, sizeof(int32), ITER_CNT);
  get_graph_throughput_by_port(sfir, "plout1", sfir.plout1, V_LEN, sizeof(int32), ITER_CNT);
	adfCheck(sfir.end(), "end sfir");
  
  return 0;
}
#endif
