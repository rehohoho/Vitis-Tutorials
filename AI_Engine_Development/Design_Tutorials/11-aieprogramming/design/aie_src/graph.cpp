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

#include "graph.h"

// instance to be compiled and used in host within xclbin
ScalarMulGraph smul;
VectorMulGraph vmul;


#if defined(__AIESIM__) || defined(__X86SIM__)
int main(int argc, char ** argv) {
	adfCheck(smul.init(), "init smul");
#ifdef __AIESIM__
  get_graph_throughput_by_port(smul, "plin1", smul.plin1, V_LEN, sizeof(int32), ITER_CNT);
#else
  adfCheck(smul.run(1), "run smul");
#endif
	adfCheck(smul.end(), "end smul");

  adfCheck(vmul.init(), "init vmul");
#ifdef __AIESIM__
  get_graph_throughput_by_port(vmul, "plin1", vmul.plin1, V_LEN, sizeof(int32), ITER_CNT);
#else
  adfCheck(vmul.run(1), "run vmul");
#endif
	adfCheck(vmul.end(), "end vmul");
  
  return 0;
}
#endif
