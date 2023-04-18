#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <adf.h>
#include "mul.h"

// for performance calculation
#define ITER_CNT 1

#define adfCheck(stmt, log_name)          \
  do {                                    \
    adf::return_code ret = stmt;          \
    if (ret != adf::ok) {                 \
      printf("[%s]: failed\n", log_name); \
    } else {                              \
      printf("[%s]: success\n", log_name);\
    }                                     \
  } while (0)


class ScalarMulGraph : public adf::graph {

  private:
    adf::kernel mul;

  public:
    adf::input_plio plin1;
    adf::input_plio plin2;
    adf::output_plio plout1;

    ScalarMulGraph() { 
      mul = adf::kernel::create(scalar_mul);
      adf::source(mul) = "mul.cc";

#ifdef EXTERNAL_IO
      plin1 = adf::input_plio::create("smul_plin1", adf::plio_64_bits);
      plin2 = adf::input_plio::create("smul_plin2", adf::plio_64_bits);
      plout1 = adf::output_plio::create("smul_plout1", adf::plio_64_bits);
#else
      plin1 = adf::input_plio::create("smul_plin1", adf::plio_64_bits, "va_10samples.txt");
      plin2 = adf::input_plio::create("smul_plin2", adf::plio_64_bits, "vb_10samples.txt");
      plout1 = adf::output_plio::create("smul_plout1", adf::plio_64_bits, "scalar_mul.txt");
#endif

      adf::connect<adf::window<V_LEN*4>>(plin1.out[0], mul.in[0]);
      adf::connect<adf::window<V_LEN*4>>(plin2.out[0], mul.in[1]);
      adf::connect<adf::window<V_LEN*4>>(mul.out[0], plout1.in[0]);
      
      adf::runtime<ratio>(mul) = 0.6;
    }

};


class VectorMulGraph : public adf::graph {

  private:
    adf::kernel mul;

  public:
    adf::input_plio plin1;
    adf::input_plio plin2;
    adf::output_plio plout1;

    VectorMulGraph() { 
      mul = adf::kernel::create(vector_mul);
      adf::source(mul) = "mul.cc";

#ifdef EXTERNAL_IO
      plin1 = adf::input_plio::create("vmul_plin1", adf::plio_64_bits);
      plin2 = adf::input_plio::create("vmul_plin2", adf::plio_64_bits);
      plout1 = adf::output_plio::create("vmul_plout1", adf::plio_64_bits);
#else
      plin1 = adf::input_plio::create("vmul_plin1", adf::plio_64_bits, "va_10samples.txt");
      plin2 = adf::input_plio::create("vmul_plin2", adf::plio_64_bits, "vb_10samples.txt");
      plout1 = adf::output_plio::create("vmul_plout1", adf::plio_64_bits, "vector_mul.txt");
#endif

      adf::connect<adf::window<V_LEN*4>>(plin1.out[0], mul.in[0]);
      adf::connect<adf::window<V_LEN*4>>(plin2.out[0], mul.in[1]);
      adf::connect<adf::window<V_LEN*4>>(mul.out[0], plout1.in[0]);
      
      adf::runtime<ratio>(mul) = 0.6;
    }

};

// uses 2/2 performance counters
void get_graph_throughput_by_port(
  adf::graph graph,
  const std::string& log_name,
  adf::IoAttr& plio_port, 
  const size_t window_len, 
  const size_t sample_bytesize,
  const size_t iter_cnt
) {
  adf::event::handle ehdl = adf::event::start_profiling(plio_port, adf::event::io_stream_start_to_bytes_transferred_cycles, window_len * iter_cnt * sample_bytesize);

  adfCheck(graph.run(iter_cnt), "run graph");
  adfCheck(graph.wait(), "wait graph");

  if (ehdl != adf::event::invalid_handle) {
    long long cycles = adf::event::read_profiling(ehdl);
    adf::event::stop_profiling(ehdl);
    printf("%s: Cycles %lld Throughput %f samples/s\n", log_name.c_str(), cycles, (double) window_len * iter_cnt / (cycles * 1e-9)); //samples/second
  } else {
    printf("%s: ERROR: Invalid handle. Only two performance counter in a AIE-PL interface tile. Event profile is not supported for x86sim.\n", log_name.c_str());
  }
}

// uses 2/2 performance counters, run first if multiple profiles are done sequentially as counters wait till net receives first data
void get_graph_latency(
  adf::graph graph,
  const std::string& log_name,
  adf::IoAttr& plio_input_port, 
  adf::IoAttr& plio_output_port,
  const size_t iter_cnt
) {
  adf::event::handle ehdl = adf::event::start_profiling(plio_input_port, plio_output_port, adf::event::io_stream_start_difference_cycles);
  
  adfCheck(graph.run(iter_cnt), "run graph");
  adfCheck(graph.wait(), "wait graph");

  if (ehdl != adf::event::invalid_handle) {
    long long cycles = adf::event::read_profiling(ehdl);
    adf::event::stop_profiling(ehdl); // performance counter is released and cleared 
    printf("[%s]: AIE Latency: %lld cycles, %fs (@1Ghz)\n", log_name.c_str(), cycles, (double) cycles * 1e-9);
  } else {
    printf("%s: ERROR: Invalid handle. Only two performance counter in a AIE-PL interface tile. Event profile is not supported for x86sim.\n", log_name.c_str());
  }
}

// Only useful for system test, uses 2/2 performance counters
void get_graph_bandwidth_by_port(
  adf::graph graph,
  const std::string& log_name,
  adf::IoAttr& plio_input_port, 
  adf::IoAttr& plio_output_port, 
  const size_t input_window_len,
  const size_t output_window_len,
  const size_t iter_cnt
) {
  adf::event::handle in_ehdl = adf::event::start_profiling(plio_input_port, adf::event::io_total_stream_running_to_idle_cycles);
  adf::event::handle out_ehdl = adf::event::start_profiling(plio_output_port, adf::event::io_total_stream_running_to_idle_cycles);
  
  adfCheck(graph.run(iter_cnt), "run graph");
  adfCheck(graph.wait(), "wait graph");

  long long cycles;
  if (in_ehdl != adf::event::invalid_handle) {
    cycles = adf::event::read_profiling(in_ehdl);
    adf::event::stop_profiling(in_ehdl);
    printf("%s: Cycles %lld Bandwidth %f, ==1: graph faster than PLIO, < 1: input port stalled\n", log_name.c_str(), cycles, (double) input_window_len * iter_cnt / cycles);
  } else {
    printf("%s: ERROR: Invalid handle. Only two performance counter in a AIE-PL interface tile. Event profile is not supported for x86sim.\n", log_name.c_str());
  }

  if (out_ehdl != adf::event::invalid_handle) {
    cycles = adf::event::read_profiling(out_ehdl);
    adf::event::stop_profiling(out_ehdl);
    printf("%s: Cycles %lld Bandwidth %f, ==1: graph faster than PLIO, < 1: output port stalled\n", log_name.c_str(), cycles, (double) output_window_len * iter_cnt / cycles);
  } else {
    printf("%s: ERROR: Invalid handle. Only two performance counter in a AIE-PL interface tile. Event profile is not supported for x86sim.\n", log_name.c_str());
  }
}

#endif // __GRAPH_H__