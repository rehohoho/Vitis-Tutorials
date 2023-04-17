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

SimGraph simGraph;


#if defined(__AIESIM__) || defined(__X86SIM__)
int main(int argc, char ** argv) {
	adfCheck(simGraph.init(), "init graph");
  
#ifdef __AIESIM__
  get_graph_latency(simGraph, "plin1/plout1", simGraph.plin1, simGraph.plout1, ITER_CNT);
  get_graph_throughput_by_port(simGraph, "plin1", simGraph.plin1, PLIN1_LEN, sizeof(int32), ITER_CNT);
  get_graph_throughput_by_port(simGraph, "plout1", simGraph.plout1, PLOUT1_LEN, sizeof(int32), ITER_CNT);
#else
  adfCheck(simGraph.run(1), "run graph");
#endif

	adfCheck(simGraph.end(), "end graph");
  
  return 0;
}
#endif
