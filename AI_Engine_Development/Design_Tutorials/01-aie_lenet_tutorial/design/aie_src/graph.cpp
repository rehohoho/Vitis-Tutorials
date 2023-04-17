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

// for performance calculation
#define PLIN1_LEN 576 * 32
#define PLOUT1_LEN 576 * 8
#define ITER_CNT 1


#if defined(__AIESIM__) || defined(__X86SIM__)
int run_graph() {
  adf::return_code ret;
  ret = simGraph.run(ITER_CNT);
  if (ret != adf::ok) {
    printf("Run failed\n");
    return ret;
  }
  return 0;
}


// uses 2/2 performance counters
void get_graph_throughput_by_port(
  const std::string& log_name,
  adf::IoAttr& plio_port, 
  const size_t window_len, 
  const size_t sample_bytesize) 
{
  adf::event::handle ehdl = adf::event::start_profiling(plio_port, adf::event::io_stream_start_to_bytes_transferred_cycles, window_len * ITER_CNT * sample_bytesize);

  run_graph();

  long long cycles = 0;
  if (ehdl != adf::event::invalid_handle) {
    cycles = adf::event::read_profiling(ehdl);
    adf::event::stop_profiling(ehdl);
    printf("%s: Cycles %lld Throughput %f samples/s\n", log_name.c_str(), cycles, (double) window_len * ITER_CNT / (cycles * 1e-9)); //samples/second
  } else {
    printf("%s: ERROR: Invalid handle. Only two performance counter in a AIE-PL interface tile. Event profile is not supported for x86sim.\n", log_name.c_str());
  }
}


// uses 2/2 performance counters
void get_graph_latency(
  const std::string& log_name,
  adf::IoAttr& plio_input_port, 
  adf::IoAttr& plio_output_port) 
{
  adf::event::handle ehdl = adf::event::start_profiling(plio_input_port, plio_output_port, adf::event::io_stream_start_difference_cycles);
  
  run_graph(); //Data transfer starts after graph.run()
  
  if (ehdl != adf::event::invalid_handle) {
    long long cycles = adf::event::read_profiling(ehdl);
    adf::event::stop_profiling(ehdl); // performance counter is released and cleared 
    printf("[%s]: AIE Latency: %lld cycles, %fs (@1Ghz)\n", log_name.c_str(), cycles, (double) cycles * 1e-9);
  } else {
    printf("%s: ERROR: Invalid handle. Only two performance counter in a AIE-PL interface tile. Event profile is not supported for x86sim.\n", log_name.c_str());
  }
}


int main(int argc, char ** argv) {
	simGraph.init();
  
  get_graph_throughput_by_port("plin1", simGraph.plin1, PLIN1_LEN, sizeof(int32));
  get_graph_throughput_by_port("plout1", simGraph.plout1, PLOUT1_LEN, sizeof(int32));
  get_graph_latency("plin1/plout1", simGraph.plin1, simGraph.plout1);

  adf::return_code ret;
  ret = simGraph.end();
  if (ret != adf::ok){
    printf("End failed\n");
    return ret;
  }
  
  return 0;
}
#endif
