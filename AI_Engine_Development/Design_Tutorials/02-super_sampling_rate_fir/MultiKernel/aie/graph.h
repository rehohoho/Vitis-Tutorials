/*
* (c) Copyright 2021-2022 Xilinx, Inc. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/
#pragma once

#include <adf.h>
#include "system_settings.h"
#include "aie_kernels.h"

std::vector<cint16> taps = std::vector<cint16>({
    {   -82,  -253},{     0,  -204},{    11,   -35},{  -198,   273},
    {  -642,   467},{ -1026,   333},{  -927,     0},{  -226,   -73},
    {   643,   467},{   984,  1355},{   550,  1691},{     0,   647},
    {   538, -1656},{  2860, -3936},{  6313, -4587},{  9113, -2961},
    {  9582,     0},{  7421,  2411},{  3936,  2860},{  1023,  1409},
    {  -200,  -615},{     0, -1778},{   517, -1592},{   467,  -643},
    {  -192,   140},{  -882,   287},{ -1079,     0},{  -755,  -245},
    {  -273,  -198},{    22,    30},{    63,   194},{     0,   266}
});

std::vector<cint16> taps_aie(taps.rbegin(),taps.rend());

#define GetPhase(Start,Step) {\
    taps_aie[Start],taps_aie[Start+Step],taps_aie[Start+2*Step],taps_aie[Start+3*Step],\
    taps_aie[Start+4*Step],taps_aie[Start+5*Step],taps_aie[Start+6*Step],taps_aie[Start+7*Step]}

std::vector<cint16> taps4_0 = std::vector<cint16>(GetPhase(0,1));

std::vector<cint16> taps4_1 = std::vector<cint16>(GetPhase(8,1));

std::vector<cint16> taps4_2 = std::vector<cint16>(GetPhase(16,1));

std::vector<cint16> taps4_3 = std::vector<cint16>(GetPhase(24,1));

const int SHIFT = 0; // to analyze the output generated by impulses at the input
//const int SHIFT = 15; // for realistic input samples

using namespace adf;



class FIRGraph_4Kernels: public adf::graph
{
private:
    kernel k[4];

public:
    input_port in[4];
    output_port out;

    FIRGraph_4Kernels()
    {

        k[0] = kernel::create_object<SingleStream::FIR_MultiKernel_cout<NUM_SAMPLES,SHIFT>>(taps4_0);
        k[1] = kernel::create_object<SingleStream::FIR_MultiKernel_cincout<NUM_SAMPLES,SHIFT>>(taps4_1);
        k[2] = kernel::create_object<SingleStream::FIR_MultiKernel_cincout<NUM_SAMPLES,SHIFT>>(taps4_2);
        k[3] = kernel::create_object<SingleStream::FIR_MultiKernel_cin<NUM_SAMPLES,SHIFT>>(taps4_3);


        const int NChunks = 4;

        for(int i=0;i<NChunks;i++)
        {
            runtime<ratio>(k[i]) = 0.9;
            source(k[i]) = "aie_kernels/FirSingleStream.cpp";
            headers(k[i]) = {"aie_kernels/FirSingleStream.h"};
        }

        // Constraints: location of the first kernel in the cascade
        location<kernel>(k[0]) = tile(25,0);

        // Discard first elements of the stream, depending on position in the cascade
        initialization_function(k[0]) = "SingleStream::FIRinit<0>";
        initialization_function(k[1]) = "SingleStream::FIRinit<8>";
        initialization_function(k[2]) = "SingleStream::FIRinit<16>";
        initialization_function(k[3]) = "SingleStream::FIRinit<24>";


        // Cascade Connections and output connection
        for(int i=0;i<NChunks-1;i++)
        connect<cascade> (k[i].out[0],k[i+1].in[1]);
        connect<stream> (k[NChunks-1].out[0],out);


        // Input Streams connections
        for(int i=0;i<NChunks;i++)
        connect<stream>(in[i],k[i].in[0]);

    };
};


class TopGraph: public adf::graph
{
public:
    FIRGraph_4Kernels G1;

    input_plio in[4];
    output_plio out;

    TopGraph() {
        for(int i=0;i<4;i++) {
            in[i] = input_plio::create("Stream for block " + std::to_string(i), plio_64_bits,"data/PhaseIn_"+std::to_string(0)+".txt", 500);
            connect<> (in[i].out[0],G1.in[i]);
        }

        out = output_plio::create("64 bits output 0", plio_64_bits,"data/Output_0.txt", 500);
        connect<> (G1.out,out.in[0]);
    }
};
