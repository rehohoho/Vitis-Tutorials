/* (c) Copyright 2021 Xilinx, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "graph_mul.h"
#include "graph_utils.h"

// instance to be compiled and used in host within xclbin
ScalarMulGraph smul;
VectorMulGraph vmul;


#ifdef __X86SIM__
int main(int argc, char ** argv) {
	adfCheck(smul.init(), "init smul");
  adfCheck(smul.run(1), "run smul");
	adfCheck(smul.end(), "end smul");

  adfCheck(vmul.init(), "init vmul");
  adfCheck(vmul.run(1), "run vmul");
	adfCheck(vmul.end(), "end vmul");
  return 0;
}
#endif


#ifdef __AIESIM__
int main(int argc, char ** argv) {
	
	adfCheck(smul.init(), "init smul");
  get_graph_latency(smul, "plin1/plout1", smul.plin1, smul.plout1, ITER_CNT);
  get_graph_throughput_by_port(smul, "plin1", smul.plin1, V_LEN, sizeof(int32), ITER_CNT);
  get_graph_throughput_by_port(smul, "plout1", smul.plout1, V_LEN, sizeof(int32), ITER_CNT);
	adfCheck(smul.end(), "end smul");

  adfCheck(vmul.init(), "init vmul");
  get_graph_latency(vmul, "plin1/plout1", vmul.plin1, vmul.plout1, ITER_CNT);
  get_graph_throughput_by_port(vmul, "plin1", vmul.plin1, V_LEN, sizeof(int32), ITER_CNT);
  get_graph_throughput_by_port(vmul, "plout1", vmul.plout1, V_LEN, sizeof(int32), ITER_CNT);
	adfCheck(vmul.end(), "end vmul");
  
  return 0;
}
#endif
