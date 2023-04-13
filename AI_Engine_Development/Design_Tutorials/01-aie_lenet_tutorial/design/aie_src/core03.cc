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

#include <stdlib.h>
#include <stdio.h>
#include "aie_api/aie.hpp"

#include "core03.h"

void core03(
        const int RowA_tile,
        const int ColA_tile,
        const int ColB_tile,
        const int tileX_A,
        const int tileY_A,
        const int tileX_C,
        const int tileY_C,
        int8_t* A_in,
        int8_t* C_out,
        int shift
) {      


	//**********Sub-Matrix dimensions********/

	constexpr size_t sizeTileA = 4 * 8;
	constexpr size_t sizeTileB = 8 * 4;
	constexpr size_t sizeTileC = 4 * 4;


	//********** Mul Intrinsic********/

	using MMUL = aie::mmul<4, 8, 4, int8_t, int8_t>;
	

        unsigned int i,j,z;
	
	for (z=0; z<RowA_tile/2; z++) chess_loop_range(2,)
	{

		//********** Output vector ********/
		int8_t * __restrict pC1 = C_out + (      z * ColB_tile +       0) * sizeTileC;
		int8_t * __restrict pC2 = C_out + ((z + 1) * ColB_tile +       0) * sizeTileC;


		for (j=0; j<ColB_tile; j+=2) chess_loop_range(2,)
		{		   	  	
			 const int8_t * __restrict pA1 = A_in + (      z * ColA_tile +       0) * sizeTileA;
          		 const int8_t * __restrict pA2 = A_in + ((z + 1) * ColA_tile +       0) * sizeTileA;
          		 const int8_t * __restrict pB1 = B03 + (      0 * ColB_tile +       j) * sizeTileB;
          		 const int8_t * __restrict pB2 = B03 + (      0 * ColB_tile + (j + 1)) * sizeTileB;

 

          		 aie::vector<int8_t, sizeTileA> A0 = aie::load_v<sizeTileA>(pA1); pA1 += sizeTileA;

          		 aie::vector<int8_t, sizeTileA> A1 = aie::load_v<sizeTileA>(pA2); pA2 += sizeTileA;

          		 aie::vector<int8_t, sizeTileB> B0 = aie::load_v<sizeTileB>(pB1); pB1 += sizeTileB * ColB_tile;

          		 aie::vector<int8_t, sizeTileB> B1 = aie::load_v<sizeTileB>(pB2); pB2 += sizeTileB * ColB_tile;


          		 MMUL C00; C00.mul(A0, B0);
          		 MMUL C01; C01.mul(A0, B1);
          		 MMUL C10; C10.mul(A1, B0);
          		 MMUL C11; C11.mul(A1, B1);

          		for (i = 1; i < ColA_tile; ++i) chess_prepare_for_pipelining chess_loop_range(3,) {
         
	 		     A0 = aie::load_v<sizeTileA>(pA1); pA1 += sizeTileA;
          		     A1 = aie::load_v<sizeTileA>(pA2); pA2 += sizeTileA;
          		     B0 = aie::load_v<sizeTileB>(pB1); pB1 += sizeTileB * ColB_tile;
          		     B1 = aie::load_v<sizeTileB>(pB2); pB2 += sizeTileB * ColB_tile;
          		     C00.mac(A0, B0);
          		     C01.mac(A0, B1);
          		     C10.mac(A1, B0);
          		     C11.mac(A1, B1);
          		 }
			 //*********will require rearrangement of output**********//
			 //*************bsrs and shift****************************//
			 
          		 aie::store_v(pC1, C00.template to_vector<int8_t>(shift)); pC1 += sizeTileC;
          		 aie::store_v(pC1, C01.template to_vector<int8_t>(shift)); pC1 += sizeTileC;
          		 aie::store_v(pC2, C10.template to_vector<int8_t>(shift)); pC2 += sizeTileC;
          		 aie::store_v(pC2, C11.template to_vector<int8_t>(shift)); pC2 += sizeTileC;




		}
	
	}



    int8_t* ptrC_relu = C_out;
    int num_iterations = COL_B_3/tileX_C;
    int r,c;

    for(r=0;r<num_iterations;r++) {
      for(c=0;c<tileX_C;c++) {
        if (*ptrC_relu < 0) {
          *ptrC_relu = 0;
        }
        ptrC_relu++;
      }
      ptrC_relu += tileX_C * (tileY_C - 1);
    }


}

void core03_top(input_window_int32 *inA, 
                output_window_int32 *out){
	int shift = 6;
    set_sat();
    set_rnd(rnd_sym_inf);

	int tileX_A = 8;
	int tileY_A = 4;
	int tileX_C = 4;
	int tileY_C = 4;

	core03(ROW_A_3 >> 2, COL_A_3 >> 3, COL_B_3 >> 2, 
           tileX_A, tileY_A, tileX_C, tileY_C, 
           (int8_t *) inA -> ptr, 
           (int8_t *) out -> ptr, 
           shift);
}
