/*
 * Copyright (C) 2019-2022, Xilinx, Inc.
 * Copyright (C) 2022-2023, Advanced Micro Devices, Inc.
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
 */
#ifndef FIR_Halfband_Decimator_Stream_5659b522_GRAPH_H_
#define FIR_Halfband_Decimator_Stream_5659b522_GRAPH_H_

#include <adf.h>
#include "fir_decimate_hb_graph.hpp"

class FIR_Halfband_Decimator_Stream_5659b522 : public adf::graph {
   public:
    static constexpr unsigned int TP_SSR = 1;
    static constexpr unsigned int TP_PARA_DECI_POLY = 1;
    template <typename dir>
    using ssr_in_port_array = std::array<adf::port<dir>, TP_SSR * TP_PARA_DECI_POLY>;
    template <typename dir>
    using ssr_coeff_port_array = std::array<adf::port<dir>, TP_SSR>;
    template <typename dir>
    using ssr_out_port_array = std::array<adf::port<dir>, TP_SSR>;

    ssr_in_port_array<input> in;
    // No dual input
    // No coeff port
    ssr_out_port_array<output> out;
    // No dual output

    std::vector<int16> taps = {-2, 5, -9, 17, -30, 52, -98, 316, 500};
    xf::dsp::aie::fir::decimate_hb::fir_decimate_hb_graph<cint16, // TT_DATA
                                                          int16,  // TT_COEF
                                                          31,     // TP_FIR_LEN
                                                          0,      // TP_SHIFT
                                                          2,      // TP_RND
                                                          256,    // TP_INPUT_WINDOW_VSIZE
                                                          1,      // TP_CASC_LEN
                                                          0,      // TP_DUAL_IP
                                                          0,      // TP_USE_COEF_RELOAD
                                                          1,      // TP_NUM_OUTPUTS
                                                          1,      // TP_API
                                                          1,      // TP_SSR
                                                          1       // TP_PARA_DECI_POLY
                                                          >
        filter;

    FIR_Halfband_Decimator_Stream_5659b522() : filter(taps) {
        adf::kernel* filter_kernels = filter.getKernels();
        for (int i = 0; i < TP_SSR * TP_PARA_DECI_POLY; i++) {
            adf::connect<> net_in(in[i], filter.in[i]);
            // No dual input
        }
        for (int i = 0; i < TP_SSR; i++) {
            // No coeff port
            adf::connect<> net_out(filter.out[i], out[i]);
            // No dual output
        }
    }
};

#endif // FIR_Halfband_Decimator_Stream_5659b522_GRAPH_H_
