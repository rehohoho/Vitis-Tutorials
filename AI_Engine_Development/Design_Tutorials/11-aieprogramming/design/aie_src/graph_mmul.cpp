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
VmulScalarGraph vmul_scalar;
VmulVectorGraph vmul_vector;
Vmul2VectorGraph v2mul_vector;
MmulScalarGraph mmul_scalar;
MmulVectorGraph mmul_vector;


#ifdef __X86SIM__
int main(int argc, char ** argv) {
	adfCheck(mmul_aieapi.init(), "init mmul_aieapi");
  adfCheck(mmul_aieapi.run(1), "run mmul_aieapi");
	adfCheck(mmul_aieapi.end(), "end mmul_aieapi");
  
  adfCheck(vmul_scalar.init(), "init vmul_scalar");
  adfCheck(vmul_scalar.run(1), "run vmul_scalar");
	adfCheck(vmul_scalar.end(), "end vmul_scalar");

  adfCheck(vmul_vector.init(), "init vmul_vector");
  adfCheck(vmul_vector.run(1), "run vmul_vector");
	adfCheck(vmul_vector.end(), "end vmul_vector");

  adfCheck(v2mul_vector.init(), "init v2mul_vector");
  adfCheck(v2mul_vector.run(1), "run v2mul_vector");
	adfCheck(v2mul_vector.end(), "end v2mul_vector");

  adfCheck(mmul_scalar.init(), "init mmul_scalar");
  adfCheck(mmul_scalar.run(1), "run mmul_scalar");
	adfCheck(mmul_scalar.end(), "end mmul_scalar");

  adfCheck(mmul_vector.init(), "init mmul_vector");
  adfCheck(mmul_vector.run(1), "run mmul_vector");
	adfCheck(mmul_vector.end(), "end mmul_vector");
  return 0;
}
#endif


#ifdef __AIESIM__
int main(int argc, char ** argv) {
	
	adfCheck(mmul_aieapi.init(), "init mmul_aieapi");
  get_graph_throughput_by_port(mmul_aieapi, "plout[0]", mmul_aieapi.plout[0], C_LEN, sizeof(int8), ITER_CNT);
	adfCheck(mmul_aieapi.end(), "end mmul_aieapi");

  adfCheck(vmul_scalar.init(), "init vmul_scalar");
  get_graph_throughput_by_port(vmul_scalar, "plout[0]", vmul_scalar.plout[0], VMUL_M, sizeof(int16), ITER_CNT);
	adfCheck(vmul_scalar.end(), "end vmul_scalar");

  adfCheck(vmul_vector.init(), "init vmul_vector");
  get_graph_throughput_by_port(vmul_vector, "plout[0]", vmul_vector.plout[0], VMUL_M, sizeof(int16), ITER_CNT);
	adfCheck(vmul_vector.end(), "end vmul_vector");

  adfCheck(v2mul_vector.init(), "init v2mul_vector");
  get_graph_throughput_by_port(v2mul_vector, "plout[0]", v2mul_vector.plout[0], VMUL_M, sizeof(int16), ITER_CNT);
	adfCheck(v2mul_vector.end(), "end v2mul_vector");
  
  adfCheck(mmul_scalar.init(), "init mmul_scalar");
  get_graph_throughput_by_port(mmul_scalar, "plout[0]", mmul_scalar.plout[0], MMUL_C_LEN, sizeof(int16), ITER_CNT);
	adfCheck(mmul_scalar.end(), "end mmul_scalar");
  
  adfCheck(mmul_vector.init(), "init mmul_vector");
  get_graph_throughput_by_port(mmul_vector, "plout[0]", mmul_vector.plout[0], MMUL_C_LEN, sizeof(int16), ITER_CNT);
	adfCheck(mmul_vector.end(), "end mmul_vector");
  return 0;
}
#endif
