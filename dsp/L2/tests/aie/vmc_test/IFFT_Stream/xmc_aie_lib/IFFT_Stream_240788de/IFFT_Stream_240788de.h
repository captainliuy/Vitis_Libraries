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
#ifndef IFFT_Stream_240788de_GRAPH_H_
#define IFFT_Stream_240788de_GRAPH_H_

#include <adf.h>
#include "fft_ifft_dit_1ch_graph.hpp"

class IFFT_Stream_240788de : public adf::graph {
   public:
    // ports
    template <typename dir>
    using ssr_port_array = std::array<adf::port<dir>, 2>;

    ssr_port_array<input> in;
    ssr_port_array<output> out;

    xf::dsp::aie::fft::dit_1ch::fft_ifft_dit_1ch_graph<cint16, // TT_DATA
                                                       cint16, // TT_TWIDDLE
                                                       64,     // TP_POINT_SIZE
                                                       0,      // TP_FFT_NIFFT
                                                       0,      // TP_SHIFT
                                                       1,      // TP_CASC_LEN
                                                       0,      // TP_DYN_PT_SIZE
                                                       64,     // TP_WINDOW_VSIZE
                                                       1,      // TP_API
                                                       0       // TP_PARALLEL_POWER
                                                       >
        fft_graph;

    IFFT_Stream_240788de() : fft_graph() {
        for (int i = 0; i < 2; i++) {
            adf::connect<> net_in(in[i], fft_graph.in[i]);
            adf::connect<> net_out(fft_graph.out[i], out[i]);
        }
    }
};

#endif // IFFT_Stream_240788de_GRAPH_H_
