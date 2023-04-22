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

#include "graph_mmul.h"
#include "graph_utils.h"

// instance to be compiled and used in host within xclbin
MmulAieapiGraph mmul_aieapi;


#ifdef __X86SIM__
int main(int argc, char ** argv) {
	adfCheck(mmul_aieapi.init(), "init mmul_aieapi");
  adfCheck(mmul_aieapi.run(1), "run mmul_aieapi");
	adfCheck(mmul_aieapi.end(), "end mmul_aieapi");
  return 0;
}
#endif


#ifdef __AIESIM__
int main(int argc, char ** argv) {
	
	adfCheck(mmul_aieapi.init(), "init mmul_aieapi");
  get_graph_latency(mmul_aieapi, "plin[0]/plout[0]", mmul_aieapi.plin[0], mmul_aieapi.plout[0], ITER_CNT);
  get_graph_throughput_by_port(mmul_aieapi, "plin[0]", mmul_aieapi.plin[0], A_LEN, sizeof(int8), ITER_CNT);
  get_graph_throughput_by_port(mmul_aieapi, "plout[0]", mmul_aieapi.plout[0], C_LEN, sizeof(int8), ITER_CNT);
	adfCheck(mmul_aieapi.end(), "end mmul_aieapi");
  return 0;
}
#endif
