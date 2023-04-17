/* (c) Copyright 2022 Xilinx, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fstream>
#include <string>
#include <cstring>

#include "input.h"
#include "golden.h"

#include "graph.h"

#include "experimental/xrt_aie.h"
#include "experimental/xrt_kernel.h"
#include "experimental/xrt_bo.h"
#include "experimental/xrt_error.h"

#include "adf/adf_api/XRTConfig.h"

//Control Offsets for DMA HLS
#define MEM_RD_OFFSET  0x10
#define MEM_WR_OFFSET  0x1C
#define SIZE_RD_OFFSET 0x28
#define SIZE_WR_OFFSET 0x30
#define CTRL_OFFSET    0x00

#define NO_IMAGES 100
#define WORD_LENGTH 2
#define INPUT_SIZE  (0x80 * NO_IMAGES)
#define OUTPUT_SIZE (0x10 * NO_IMAGES)
#define OUT_IMAGE_LENGTH  32
//#define INPUT_SIZE_128b (INPUT_SIZE / 2)
//#define OUTPUT_SIZE_128b (OUTPUT_SIZE / 2)

// load_xclbin function is used to read in xclbin file
static std::vector<char>
load_xclbin(xrtDeviceHandle device, const std::string &fnm)
{
   if (fnm.empty()) {
      throw std::runtime_error("No xclbin specified");
   }
   
   // load bit stream
   std::ifstream stream(fnm);
   stream.seekg(0,stream.end);
   size_t size = stream.tellg();
   stream.seekg(0,stream.beg);
   
   std::vector<char> header(size);
   stream.read(header.data(),size);
   
   auto top = reinterpret_cast<const axlf*>(header.data());
   if (xrtDeviceLoadXclbin(device, top)) {
      throw std::runtime_error("Bitstream download failed");
   }
   
   return header;
}

int main(int argc, char ** argv)
{
   size_t input_size_in_bytes = INPUT_SIZE * sizeof(uint32_t)* WORD_LENGTH;
   size_t output_size_in_bytes = OUTPUT_SIZE * sizeof(uint32_t)* WORD_LENGTH;
   
   //////////////////////////////////////////
   // Open xclbin
   //////////////////////////////////////////
   
   if((argc < 2) || (argc > 3)) {
      std::cout << "Usage: " << argv[0] <<" <xclbin>" << "iteration count(optional)" << std::endl;
      return EXIT_FAILURE;
   }
   
   const char* xclbinFilename = argv[1];
   
   int16_t iterCnt = 0;
   if(argc == 3) {
      std::string iter = argv[2];
      iterCnt = stoi(iter);
   }
   else {
      iterCnt = LENET_INSTS;
   }
   
   printf("Iteration : %d...\n", iterCnt);

   auto deviceIdx = xrt::device(0);
   auto dhdl = xrtDeviceOpen(0);
   auto xclbin = load_xclbin(dhdl, xclbinFilename);
   auto top = reinterpret_cast<const axlf*>(xclbin.data());

#ifndef EXTERNAL_IO
   //Allocate BOs (buffer objects) of requested size with appropriate flags
   //Memory map BOs into user's address space (DDR Memory)
   xrtBufferHandle in_bohdl = xrtBOAlloc(dhdl, input_size_in_bytes, 0, 0);
   auto in_bomapped = reinterpret_cast<uint32_t*>(xrtBOMap(in_bohdl));
   
   //Set the input mapped region needs to have the same data as the input_pl. 
   memcpy(in_bomapped, input_pl, input_size_in_bytes);
   printf("Input memory virtual addr 0x%p\n", in_bomapped);
   
   xrtBufferHandle out_bohdl = xrtBOAlloc(dhdl, output_size_in_bytes, 0, 0);
   auto out_bomapped = reinterpret_cast<uint32_t*>(xrtBOMap(out_bohdl));
   printf("Output memory virtual addr 0x%p\n", out_bomapped);
   
   //open PL kernels and obtain handles
   xrtKernelHandle dmahls_khdl = xrtPLKernelOpen(dhdl, top->m_header.uuid, "dma_hls:{dma_hls_0}");
   
   //Create a kernel handle to start DMA HLS pl kernel
   //Set the DMA HLS arguments (input buffer handle, output
   //buffer handle, input size and output size) 
   // 0,1,4,5 are port number on the IP.
   xrtRunHandle dmahls_rhdl = xrtRunOpen(dmahls_khdl); 
   int rval = xrtRunSetArg(dmahls_rhdl, 0, in_bohdl);
   rval     = xrtRunSetArg(dmahls_rhdl, 1, out_bohdl);
   rval     = xrtRunSetArg(dmahls_rhdl, 4, INPUT_SIZE);
   rval     = xrtRunSetArg(dmahls_rhdl, 5, OUTPUT_SIZE);
   rval     = xrtRunSetArg(dmahls_rhdl, 6, iterCnt);

   //Start the DMA HLS kernel
   //Moving data from DDR to AI Engine  
   xrtRunStart(dmahls_rhdl);
   printf("Run Datamover\n");
#endif
   
   //////////////////////////////////////////
   // graph execution for AIE
   //////////////////////////////////////////
   
   adf::registerXRT(dhdl, top->m_header.uuid);
   adf::return_code ret;

   SimGraph simGraph;

   try {
      adfCheck(simGraph.init(), "init graph");

#ifndef EXTERNAL_IO
      get_graph_latency(simGraph, "plin1/plout1", simGraph.plin1, simGraph.plout1, iterCnt*100);
      get_graph_throughput_by_port(simGraph, "plin1", simGraph.plin1, PLIN1_LEN, sizeof(int32), iterCnt*100);
      get_graph_throughput_by_port(simGraph, "plout1", simGraph.plout1, PLOUT1_LEN, sizeof(int32), iterCnt*100);
#else
      get_graph_latency(simGraph, "plin1/plout1", simGraph.plin1, simGraph.plout1, iterCnt);
      get_graph_throughput_by_port(simGraph, "plin1", simGraph.plin1, PLIN1_LEN, sizeof(int32), iterCnt);
      get_graph_throughput_by_port(simGraph, "plout1", simGraph.plout1, PLOUT1_LEN, sizeof(int32), iterCnt);
#endif
      
      adfCheck(simGraph.end(), "end graph");
   }

   catch (const std::system_error& ex) {
      xrt::error error(deviceIdx, XRT_ERROR_CLASS_AIE);
      auto errCode = error.get_error_code();
      auto timestamp = error.get_timestamp();
      auto err_str = error.to_string();
      std::cout << timestamp << " error code:" << errCode << " Error:" << err_str << std::endl;
   }
   
#ifndef EXTERNAL_IO
   //Wait for DMA HLS execution to finish
   printf("Waiting for dma hls to complete...\n");
   auto state = xrtRunWait(dmahls_rhdl);
   std::cout << "Datamover completed with status(" << state << ")\n";
   
   //Close the run handles obtained by in xrtRunOpen() 
   xrtRunClose(dmahls_rhdl);
   
   //Close opened kernel handles obtained from xrtPLKernelOpen()
   xrtKernelClose(dmahls_khdl);
   
   printf("Closed dma hls kernel...\n");
   
   //Compare data in out_bomapped to golden data in golden.h
   int errCnt = 0;
   int errFlag = 0;
   int out_count = 0;
   int image_count = 0;
   
   printf("Comparing output with golden\n");
   for(int i = 0; i < OUTPUT_SIZE * WORD_LENGTH; i++) {
       if( out_bomapped[i] == int32golden[i] ) {
           errFlag = errFlag || 0;
           ++out_count;
       } else {
           errFlag = errFlag || 1;
           printf("Error found in sample 0x%X != to the golden 0x%X\n", out_bomapped[i], int32golden[i] );
           ++errCnt;
       }
       if ( (out_count == OUT_IMAGE_LENGTH) ) {
           if (!errFlag) {
               printf("Pass for image %d \n", image_count);
               ++image_count;
               out_count = 0;
               errFlag = 0;
           } else {
               printf("Fail for image %d \n", image_count);
               ++image_count;
               out_count = 0;
               errFlag = 0;
           }
       }
   }
   
   //Release allocated resources
   std::cout << "Releasing remaining XRT objects...\n";
   xrtBOFree(in_bohdl);
   xrtBOFree(out_bohdl);
#endif
   
   xrtDeviceClose(dhdl);
   
#ifndef EXTERNAL_IO
   std::cout << "TEST " << (errCnt ? "FAILED" : "PASSED") << std::endl;
   return (errCnt ? EXIT_FAILURE :  EXIT_SUCCESS);
#else
   return 0;
#endif
   
}
