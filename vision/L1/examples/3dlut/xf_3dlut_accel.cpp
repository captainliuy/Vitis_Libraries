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

#include "xf_3dlut_accel_config.h"

static constexpr int __XF_DEPTH_IN = (HEIGHT * WIDTH * (XF_PIXELWIDTH(IN_TYPE, NPPCX)) / 8) / (INPUT_PTR_WIDTH / 8);
static constexpr int __XF_DEPTH_LUT =
    (SQ_LUTDIM * LUT_DIM * (XF_PIXELWIDTH(XF_32FC3, NPPCX)) / 8) / (LUT_PTR_WIDTH / 8) + 1;
static constexpr int __XF_DEPTH_OUT = (HEIGHT * WIDTH * (XF_PIXELWIDTH(OUT_TYPE, NPPCX)) / 8) / (OUTPUT_PTR_WIDTH / 8);

void lut3d_accel(ap_uint<INPUT_PTR_WIDTH>* img_in,
                 ap_uint<OUTPUT_PTR_WIDTH>* img_out,
                 ap_uint<LUT_PTR_WIDTH>* lut,
                 int height,
                 int width,
                 int lutDim) {
// clang-format off
    #pragma HLS INTERFACE m_axi      port=img_in        offset=slave  bundle=gmem0 depth=__XF_DEPTH_IN
	#pragma HLS INTERFACE m_axi      port=lut        offset=slave  bundle=gmem1 depth=__XF_DEPTH_LUT
    #pragma HLS INTERFACE m_axi      port=img_out       offset=slave  bundle=gmem2 depth=__XF_DEPTH_OUT
    #pragma HLS INTERFACE s_axilite  port=height
    #pragma HLS INTERFACE s_axilite  port=width
	#pragma HLS INTERFACE s_axilite  port=lutDim
    #pragma HLS INTERFACE s_axilite  port=return
    // clang-format on

    xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, NPPCX, XF_CV_DEPTH_IN_1> imgInput(height, width);
    xf::cv::Mat<XF_32FC3, SQ_LUTDIM, LUT_DIM, NPPCX, XF_CV_DEPTH_IN_2> lutMat(lutDim * lutDim, lutDim);
    xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPPCX, XF_CV_DEPTH_OUT_1> imgOutput(height, width);

#pragma HLS DATAFLOW

    // Retrieve xf::cv::Mat objects from img_in, lut data:
    xf::cv::Array2xfMat<INPUT_PTR_WIDTH, IN_TYPE, HEIGHT, WIDTH, NPPCX, XF_CV_DEPTH_IN_1>(img_in, imgInput);
    xf::cv::Array2xfMat<LUT_PTR_WIDTH, XF_32FC3, SQ_LUTDIM, LUT_DIM, NPPCX, XF_CV_DEPTH_IN_2>(lut, lutMat);

    // Run xfOpenCV kernel:
    xf::cv::lut3d<LUT_DIM, SQ_LUTDIM, IN_TYPE, OUT_TYPE, HEIGHT, WIDTH, NPPCX, XF_USE_URAM, XF_CV_DEPTH_IN_1,
                  XF_CV_DEPTH_IN_2, XF_CV_DEPTH_OUT_1>(imgInput, lutMat, imgOutput, lutDim);

    // Convert _dst xf::cv::Mat object to output array:
    xf::cv::xfMat2Array<OUTPUT_PTR_WIDTH, OUT_TYPE, HEIGHT, WIDTH, NPPCX, XF_CV_DEPTH_OUT_1>(imgOutput, img_out);

    return;
} // End of kernel