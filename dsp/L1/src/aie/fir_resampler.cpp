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
/*
FIR kernel code.
This file captures the body of run-time code for the kernal class.

Coding conventions
  TT_      template type suffix
  TP_      template parameter suffix
*/

#pragma once
#include <adf.h>

#define __AIE_API_USE_NATIVE_1024B_VECTOR__

#include "aie_api/aie_adf.hpp"

#include "kernel_api_utils.hpp"

#include "fir_resampler.hpp"
#include "fir_resampler_utils.hpp"
//#define _DSPLIB_FIR_RESAMPLER_HPP_DEBUG_

#include <cmath> // For power function

// According to template parameter the input may be a window, or window and cascade input
// Similarly the output interface may be a window or a cascade output
//-----------------------------------------------------------------------------------------------------

namespace xf {
namespace dsp {
namespace aie {
namespace fir {
namespace resampler {

template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          bool TP_CASC_IN,
          bool TP_CASC_OUT,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN,
          unsigned int TP_USE_COEFF_RELOAD,
          unsigned int TP_NUM_OUTPUTS,
          unsigned int TP_DUAL_IP,
          unsigned int TP_API>
void kernelFilterClass<TT_DATA,
                       TT_COEFF,
                       TP_FIR_LEN,
                       TP_INTERPOLATE_FACTOR,
                       TP_DECIMATE_FACTOR,
                       TP_SHIFT,
                       TP_RND,
                       TP_INPUT_WINDOW_VSIZE,
                       TP_CASC_IN,
                       TP_CASC_OUT,
                       TP_FIR_RANGE_LEN,
                       TP_KERNEL_POSITION,
                       TP_CASC_LEN,
                       TP_USE_COEFF_RELOAD,
                       TP_NUM_OUTPUTS,
                       TP_DUAL_IP,
                       TP_API>::filterKernel(T_inputIF<TP_CASC_IN, TT_DATA, TP_DUAL_IP> inInterface,
                                             T_outputIF<TP_CASC_OUT, TT_DATA> outInterface) {
    windowBroadcast<TT_DATA,
                    TP_INPUT_WINDOW_VSIZE +
                        fnFirMargin<((TP_FIR_LEN + TP_INTERPOLATE_FACTOR - 1) / TP_INTERPOLATE_FACTOR), TT_DATA>(),
                    TP_API, TP_DUAL_IP>(inInterface, outInterface);
    filterSelectArch(inInterface, outInterface);
}

// Asymmetric Fractional Interpolation FIR Kernel Function - overloaded (not specialised)
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          bool TP_CASC_IN,
          bool TP_CASC_OUT,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN,
          unsigned int TP_USE_COEFF_RELOAD,
          unsigned int TP_NUM_OUTPUTS,
          unsigned int TP_DUAL_IP,
          unsigned int TP_API>
INLINE_DECL void kernelFilterClass<TT_DATA,
                                   TT_COEFF,
                                   TP_FIR_LEN,
                                   TP_INTERPOLATE_FACTOR,
                                   TP_DECIMATE_FACTOR,
                                   TP_SHIFT,
                                   TP_RND,
                                   TP_INPUT_WINDOW_VSIZE,
                                   TP_CASC_IN,
                                   TP_CASC_OUT,
                                   TP_FIR_RANGE_LEN,
                                   TP_KERNEL_POSITION,
                                   TP_CASC_LEN,
                                   TP_USE_COEFF_RELOAD,
                                   TP_NUM_OUTPUTS,
                                   TP_DUAL_IP,
                                   TP_API>::filterKernel(T_inputIF<TP_CASC_IN, TT_DATA, TP_DUAL_IP> inInterface,
                                                         T_outputIF<TP_CASC_OUT, TT_DATA> outInterface,
                                                         const TT_COEFF (&inTaps)[TP_FIR_LEN]) {
    windowBroadcast<TT_DATA,
                    TP_INPUT_WINDOW_VSIZE +
                        fnFirMargin<((TP_FIR_LEN + TP_INTERPOLATE_FACTOR - 1) / TP_INTERPOLATE_FACTOR), TT_DATA>(),
                    TP_API, TP_DUAL_IP>(inInterface, outInterface);
    m_coeffnEq = rtpCompare(inTaps, m_oldInTaps);

    sendRtpTrigger(m_coeffnEq, outInterface);
    if (m_coeffnEq) { // Coefficients have changed
        bufferReload(inTaps, m_oldInTaps, outInterface);
        firReload(inTaps);
        chess_memory_fence();
    }

    filterSelectArch(inInterface, outInterface);
}

// Asymmetric Fractional Interpolation FIR Kernel Function - overloaded (not specialised)
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          bool TP_CASC_IN,
          bool TP_CASC_OUT,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN,
          unsigned int TP_USE_COEFF_RELOAD,
          unsigned int TP_NUM_OUTPUTS,
          unsigned int TP_DUAL_IP,
          unsigned int TP_API>
INLINE_DECL void kernelFilterClass<TT_DATA,
                                   TT_COEFF,
                                   TP_FIR_LEN,
                                   TP_INTERPOLATE_FACTOR,
                                   TP_DECIMATE_FACTOR,
                                   TP_SHIFT,
                                   TP_RND,
                                   TP_INPUT_WINDOW_VSIZE,
                                   TP_CASC_IN,
                                   TP_CASC_OUT,
                                   TP_FIR_RANGE_LEN,
                                   TP_KERNEL_POSITION,
                                   TP_CASC_LEN,
                                   TP_USE_COEFF_RELOAD,
                                   TP_NUM_OUTPUTS,
                                   TP_DUAL_IP,
                                   TP_API>::filterKernelRtp(T_inputIF<TP_CASC_IN, TT_DATA, TP_DUAL_IP> inInterface,
                                                            T_outputIF<TP_CASC_OUT, TT_DATA> outInterface) {
    windowBroadcast<TT_DATA,
                    TP_INPUT_WINDOW_VSIZE +
                        fnFirMargin<((TP_FIR_LEN + TP_INTERPOLATE_FACTOR - 1) / TP_INTERPOLATE_FACTOR), TT_DATA>(),
                    TP_API, TP_DUAL_IP>(inInterface, outInterface);
    m_coeffnEq = getRtpTrigger(); // 0 - equal, 1 - not equal

    sendRtpTrigger(m_coeffnEq, outInterface);
    if (m_coeffnEq) { // Coefficients have changed
        bufferReload<TT_DATA, TT_COEFF, TP_FIR_LEN>(inInterface, m_oldInTaps, outInterface);
        firReload(m_oldInTaps);
        chess_memory_fence();
    }

    filterSelectArch(inInterface, outInterface);
}

// Asymmetric Fractional Interpolation FIR Kernel Function - overloaded (not specialised)
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          bool TP_CASC_IN,
          bool TP_CASC_OUT,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN,
          unsigned int TP_USE_COEFF_RELOAD,
          unsigned int TP_NUM_OUTPUTS,
          unsigned int TP_DUAL_IP,
          unsigned int TP_API>
INLINE_DECL void kernelFilterClass<TT_DATA,
                                   TT_COEFF,
                                   TP_FIR_LEN,
                                   TP_INTERPOLATE_FACTOR,
                                   TP_DECIMATE_FACTOR,
                                   TP_SHIFT,
                                   TP_RND,
                                   TP_INPUT_WINDOW_VSIZE,
                                   TP_CASC_IN,
                                   TP_CASC_OUT,
                                   TP_FIR_RANGE_LEN,
                                   TP_KERNEL_POSITION,
                                   TP_CASC_LEN,
                                   TP_USE_COEFF_RELOAD,
                                   TP_NUM_OUTPUTS,
                                   TP_DUAL_IP,
                                   TP_API>::filterSelectArch(T_inputIF<TP_CASC_IN, TT_DATA, TP_DUAL_IP> inInterface,
                                                             T_outputIF<TP_CASC_OUT, TT_DATA> outInterface) {
    // Display constants for debug

    windowReset<TT_DATA, TP_CASC_IN, TP_DUAL_IP, TP_API>(inInterface);
    if
        constexpr(m_kArch == kArchStream) { filterStream(inInterface, outInterface); }
    else {
        filterBasic(inInterface, outInterface);
    }
}

template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          bool TP_CASC_IN,
          bool TP_CASC_OUT,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN,
          unsigned int TP_USE_COEFF_RELOAD,
          unsigned int TP_NUM_OUTPUTS,
          unsigned int TP_DUAL_IP,
          unsigned int TP_API>
void kernelFilterClass<TT_DATA,
                       TT_COEFF,
                       TP_FIR_LEN,
                       TP_INTERPOLATE_FACTOR,
                       TP_DECIMATE_FACTOR,
                       TP_SHIFT,
                       TP_RND,
                       TP_INPUT_WINDOW_VSIZE,
                       TP_CASC_IN,
                       TP_CASC_OUT,
                       TP_FIR_RANGE_LEN,
                       TP_KERNEL_POSITION,
                       TP_CASC_LEN,
                       TP_USE_COEFF_RELOAD,
                       TP_NUM_OUTPUTS,
                       TP_DUAL_IP,
                       TP_API>::filterBasic(T_inputIF<TP_CASC_IN, TT_DATA, TP_DUAL_IP> inInterface,
                                            T_outputIF<TP_CASC_OUT, TT_DATA> outInterface) {
    set_rnd(TP_RND);
    set_sat();

    static_assert(windowDecPhase[0] % (params.alignWindowReadBytes / params.dataSizeBytes) == 0,
                  "ERROR: Solution doesn't meet alignment requirements. Window decrements must be aligned to 128b "
                  "boundary. Increase m_kPolyphaseLaneAlias usually solves this. ");
    static_assert(windowDecPhase[m_kPolyphaseLaneAlias - 1] % (params.alignWindowReadBytes / params.dataSizeBytes) == 0,
                  "ERROR: Solution doesn't meet alignment requirements. Window decrements must be aligned to 128b "
                  "boundary. Increase m_kPolyphaseLaneAlias usually solves this. ");

    T_buff_256b<TT_COEFF>* restrict coeff = (T_buff_256b<TT_COEFF>*)m_internalTaps;
    T_buff_256b<TT_COEFF> coe0; // register for coeff values.
    T_buff_1024b<TT_DATA> sbuff;
    T_acc<TT_DATA, TT_COEFF> acc;
    T_outVal<TT_DATA, TT_COEFF> outVal;
    T_buff_256b<TT_DATA> readData; // input data read from window, bound for sbuff
    unsigned int dataNeeded;
    unsigned int xstart = 0;

    TT_COEFF* m_internalTapsCopy = (TT_COEFF*)m_internalTaps; // points to m_internalTaps[0][0][0][0]

    constexpr int k128Vsize = 128 / 8 / sizeof(TT_DATA);
    using t_128vect = ::aie::vector<TT_DATA, k128Vsize>;
    t_128vect* read128Ptr;
    constexpr int k256Vsize = 256 / 8 / sizeof(TT_DATA);
    using t_256vect = ::aie::vector<TT_DATA, k256Vsize>;
    t_256vect* read256Ptr;

    auto inItr = (TP_API == USE_WINDOW_API && TP_KERNEL_POSITION != 0)
                     ? ::aie::begin_random_circular(*(inInterface.inWindowLin))
                     : ::aie::begin_random_circular(*(inInterface.inWindowCirc));

    constexpr bool hasOutWindow = (TP_API == 0 && TP_KERNEL_POSITION == TP_CASC_LEN - 1);
    constexpr bool hasOutWindow2 = (TP_NUM_OUTPUTS == 2 && TP_API == 0 && TP_KERNEL_POSITION == TP_CASC_LEN - 1);
    auto outItr = cond_begin_vector_random_or_scalar_circular<hasOutWindow, m_kVOutSize>(*outInterface.outWindow);
    auto outItr2 = cond_begin_vector_random_or_scalar_circular<hasOutWindow2, m_kVOutSize>(*outInterface.outWindow2);

    // Move data pointer away from data consumed by previous cascades
    // Move only by  multiples of 128bit. Cascade phase remainder goes to m_kDataBuffXOffset
    inItr += (TRUNC((m_kFirInitOffset), (m_kWinAccessByteSize / sizeof(TT_DATA))));

    // Incremental loads cause a very un-wanted new loop (strobeFactor) because upd_w idx has to be compile time
    // constant
    // this essentially puts a requirement of having at least strobeFactor*PhaseLaneAlias*FirLen window length i think
    // numDataLoads =0;
    // dataLoaded = 0;
    // ideally only add this once
    // dataNeeded = m_kDataBuffXOffset;
    // This loop creates the output window data. In each iteration a vector of samples is output
    for (unsigned i = 0; i < m_kLsize; i++) chess_prepare_for_pipelining chess_loop_range(m_kLsize, ) {
// Allows us to keep upd_w as compile tile constant
// HAZARD : The splice in buffer is unlikely to line up very well,
// but right now, we don't use more than dataLoadSize for a given op
// dataLoaded = 0;
// numDataLoads =0;
// How many operations until the 0th polyphase is the first lane again.
#pragma unroll(m_kPolyphaseLaneAlias)
            for (unsigned offsetPhase = 0; offsetPhase < m_kPolyphaseLaneAlias; ++offsetPhase) {
                // CRVO-3835 - force to be loop-invariant - shouldn't result in any extra microcode, but gives the
                // compiler a hint
                m_internalTapsCopy = chess_copy(m_internalTapsCopy);
                // Reset coeff pointer back to appropriate starting position depending on which phase we are in
                // effectively indexing m_internalTaps[offsetPhase%m_kPolyphaseCoeffAlias][op/m_kColumns][0][0][0]
                coeff = ((T_buff_256b<TT_COEFF>*)m_internalTapsCopy) +
                        (offsetPhase % m_kPolyphaseCoeffAlias) * m_kNumOps * m_kMultsPerCycle / m_kCoeffRegVsize;
                int coeffPhaseStart =
                    (offsetPhase % m_kPolyphaseCoeffAlias) * m_kNumOps * m_kMultsPerCycle % m_kCoeffRegVsize;

                // 256b increment is the correct granularity because TT_COEFF*nLanes*nColumns will always be 256b.
                coe0 = *coeff++;

                // Preamble, calculate and load data from window into register
                // A given offsetPhase may require more samples than another.
                // It shouldn't make a difference for ops/load, but accountancy might skew if we take the larger
                xstart = xstartPhase[offsetPhase]; //+m_kDataBuffXOffset (i've inadvertnly already added this)
                dataNeeded = m_kDataNeededPhase[offsetPhase] + xstart; // Adding the xstart to show that some samples
                                                                       // that are loaded are not used at all, but we
                                                                       // need them anyway.
                // dataNeeded = m_kDataBuffXOffset + m_kVOutSize + m_kColumns-1;

                // pragma unroll needs completely constant, but offsetPhase doesn't look constant (even though it will
                // be due to pahse unroll..)
                //#pragma unroll (m_kInitialLoads[offsetPhase])
                for (int initLoads = 0; initLoads < m_kInitialLoads[offsetPhase]; ++initLoads) {
                    read256Ptr = (t_256vect*)&*inItr;
                    inItr += k256Vsize;
                    readData.val = *read256Ptr;
                    sbuff.val = upd_w(sbuff.val, initLoads % m_kDataLoadsInReg,
                                      readData.val); // Update sbuff with data from input window. 00++|____|____|____
                }
                // Ensures that these can be treated as compile time constant in the next unrolled loop.
                unsigned int dataLoaded = m_kDataLoadVsize * m_kInitialLoads[offsetPhase];
                unsigned int numDataLoads = m_kInitialLoads[offsetPhase];
                // Read cascade input. Do nothing if cascade input not present.
                acc = readCascade(inInterface, acc);
                // Init Vector operation. VMUL if cascade not present, otherwise VMAC
                acc = initMacResampler(inInterface, acc, sbuff, xstart, xoffsets[offsetPhase], coe0, coeffPhaseStart,
                                       zoffsets[offsetPhase]);

#pragma unroll(GUARD_ZERO((m_kNumOps - 1)))
                for (int op = m_kColumns; op < m_kFirLenCeilCols; op += m_kColumns) {
                    int coeffStart = (coeffPhaseStart + (op / m_kColumns) * m_kMultsPerCycle) % m_kCoeffRegVsize;
                    dataNeeded += m_kColumns;

                    if (dataNeeded > dataLoaded) {
                        read256Ptr = (t_256vect*)&*inItr;
                        inItr += k256Vsize;
                        readData.val = *read256Ptr;
                        sbuff.val = upd_w(sbuff.val, (numDataLoads % m_kDataLoadsInReg),
                                          readData.val); // Update sbuff with data from input window
                        dataLoaded += m_kDataLoadVsize;
                        numDataLoads++;
                    }
                    // Only load the next chunk of coefficients once we've wrapped round the current buffer.
                    if (coeffStart == 0) {
                        coe0 = *coeff++;
                    }
                    acc = macResampler(acc, sbuff, (op + xstart), xoffsets[offsetPhase], coe0, coeffStart,
                                       zoffsets[offsetPhase]);
                }
                // Use dataNeededPhase to show how many data samples were consumed
                inItr -= windowDecPhase[offsetPhase]; // return read pointer to start of next chunk of window.

                // Write cascade. Do nothing if cascade not present.
                writeCascade<TT_DATA, TT_COEFF>(outInterface, acc);

                outVal = shiftAndSaturate(acc, TP_SHIFT);

                // Write to output window
                writeOutput<TT_DATA, TT_COEFF, TP_NUM_OUTPUTS, TP_API>(outInterface, outVal,
                                                                       0 /*stream phase - irrelevant here */, outItr);
                if
                    constexpr(TP_NUM_OUTPUTS == 2 && TP_API == 0) {
                        writeOutput<TT_DATA, TT_COEFF, TP_NUM_OUTPUTS, TP_API>(
                            outInterface, outVal, 0 /*stream phase - irrelevant here */, outItr2);
                    }
            } // m_kPolyphaseLaneAlias

        } // LSize
};

template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          bool TP_CASC_IN,
          bool TP_CASC_OUT,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN,
          unsigned int TP_USE_COEFF_RELOAD,
          unsigned int TP_NUM_OUTPUTS,
          unsigned int TP_DUAL_IP,
          unsigned int TP_API>
void kernelFilterClass<TT_DATA,
                       TT_COEFF,
                       TP_FIR_LEN,
                       TP_INTERPOLATE_FACTOR,
                       TP_DECIMATE_FACTOR,
                       TP_SHIFT,
                       TP_RND,
                       TP_INPUT_WINDOW_VSIZE,
                       TP_CASC_IN,
                       TP_CASC_OUT,
                       TP_FIR_RANGE_LEN,
                       TP_KERNEL_POSITION,
                       TP_CASC_LEN,
                       TP_USE_COEFF_RELOAD,
                       TP_NUM_OUTPUTS,
                       TP_DUAL_IP,
                       TP_API>::filterStream(T_inputIF<TP_CASC_IN, TT_DATA, TP_DUAL_IP> inInterface,
                                             T_outputIF<TP_CASC_OUT, TT_DATA> outInterface) {
    set_rnd(TP_RND);
    set_sat();

    static_assert(windowDecPhase[0] % (params.alignWindowReadBytes / params.dataSizeBytes) == 0,
                  "ERROR: Solution doesn't meet alignment requirements. Window decrements must be aligned to 128b "
                  "boundary. Increase m_kPolyphaseLaneAlias usually solves this. ");
    static_assert(windowDecPhase[m_kPolyphaseLaneAlias - 1] % (params.alignWindowReadBytes / params.dataSizeBytes) == 0,
                  "ERROR: Solution doesn't meet alignment requirements. Window decrements must be aligned to 128b "
                  "boundary. Increase m_kPolyphaseLaneAlias usually solves this. ");

    T_buff_256b<TT_COEFF>* __restrict coeff = (T_buff_256b<TT_COEFF>*)m_internalTaps;
    T_buff_256b<TT_COEFF> coe0, coe1, coe2;                           // register for coeff values.
    T_buff_1024b<TT_DATA>* ptr_delay = (T_buff_1024b<TT_DATA>*)delay; // heap storage pointer
    T_buff_1024b<TT_DATA> sbuff = *ptr_delay; // initialize data register with data allocated on heap
    T_acc384<TT_DATA, TT_COEFF> acc;
    T_outVal384<TT_DATA, TT_COEFF> outVal;
    T_buff_256b<TT_DATA> readData; // input data read from window, bound for sbuff
    unsigned int dataNeeded, dataLoaded;
    unsigned int inDataLoadPhase, outDataPhase = 0;
    unsigned int xstart = 0;
    unsigned int numDataLoads = 0;
    static constexpr int dataNeededLastKernel =
        (1 + TP_DECIMATE_FACTOR * (m_kLanes - 1) + (streamInitNullAccs * TP_DECIMATE_FACTOR * m_kLanes));
    static constexpr int dataOffsetNthKernel =
        (m_kFirLenCeil - TP_FIR_RANGE_LEN - (TP_INTERPOLATE_FACTOR * m_kFirRangeOffset));
    static constexpr unsigned int kMinDataNeeded = (dataNeededLastKernel - dataOffsetNthKernel);
    static constexpr unsigned int kMinDataLoadCycles =
        (CEIL(kMinDataNeeded, (TP_INTERPOLATE_FACTOR * m_kStreamLoadVsize))) /
        (TP_INTERPOLATE_FACTOR * m_kStreamLoadVsize);

    int loopSize = (m_kLsize / m_kRepeatFactor);
    //  Stream initialization's Null Accs m_kPolyphaseLaneAlias Remainder
    static constexpr int dataReadInitPhase =
        (TP_DECIMATE_FACTOR * (m_kRepeatFactor * m_kPolyphaseLaneAlias - streamInitNullAccs - 1) * m_kVOutSize +
         kMinDataNeeded);
    static constexpr int numberLoadsInitPhase =
        CEIL((dataReadInitPhase), (TP_INTERPOLATE_FACTOR * m_kStreamLoadVsize)) /
        (TP_INTERPOLATE_FACTOR * m_kStreamLoadVsize);
    int startDataLoads = marginLoadsMappedToBuff + numberLoadsInitPhase;
    int startDataOffset = (streamDataOffsetWithinBuff) % m_kSamplesInBuff;

    TT_COEFF* m_internalTapsCopy = (TT_COEFF*)m_internalTaps; // points to m_internalTaps[0][0][0][0]
    // Init pre-loop to deal with m_kFirInitOffset. Only generate for cascaded designs
    if
        constexpr(TP_CASC_LEN > 1) {
            if (doInit == 1) {
                // Initial stage requires 3 separate scenarios to be considered.
                // 1. Init Null Macs.
                //    Send Null Cascade words to skew cascaded kernels in such way, that all operate on same input data
                // 2. CEIL(streamInitNullAccs, m_kPolyphaseLaneAlias)
                //    Ceil up to m_kPolyphaseLaneAlias,
                // 3. streamInitAccStrobes
                //    Ceil up to m_kRepeatFactor, so that data buffer is fully "used" and first inner loop
                //    iteration starts in the same place as further kernel execution iterations.
                // The above must produce N * m_kRepeatFactor partial output vectors, i.e. cascade writes,
                // so the main inner loop can be unrolled, loop count decremented by N
                // and correct total number of samples operated on.
                for (unsigned i = 0; i < streamInitNullAccs; i++)
                    chess_prepare_for_pipelining chess_loop_range(streamInitNullAccs, ) {
                        acc = readCascade(inInterface, acc);

                        writeCascade<TT_DATA, TT_COEFF>(outInterface, acc);
                    }

                // dataNeeded = 0;
                dataNeeded = kMinDataNeeded;

                dataLoaded = 0;
                inDataLoadPhase = 0;
                numDataLoads = marginLoadsMappedToBuff;

#pragma unroll(GUARD_ZERO(CEIL(streamInitNullAccs, m_kPolyphaseLaneAlias) - streamInitNullAccs))
                for (unsigned streamInitNullMacPhase = streamInitNullAccs;
                     streamInitNullMacPhase < CEIL(streamInitNullAccs, m_kPolyphaseLaneAlias);
                     streamInitNullMacPhase++) {
                    m_internalTapsCopy = chess_copy(m_internalTapsCopy);
                    int offsetPhase = streamInitNullMacPhase % m_kPolyphaseLaneAlias;
                    int strobe = streamInitNullMacPhase / m_kPolyphaseLaneAlias;

                    coeff = ((T_buff_256b<TT_COEFF>*)m_internalTapsCopy) +
                            (offsetPhase % m_kPolyphaseCoeffAlias) * m_kNumOps * m_kMultsPerCycle / m_kCoeffRegVsize;
                    int coeffPhaseStart =
                        (offsetPhase % m_kPolyphaseCoeffAlias) * m_kNumOps * m_kMultsPerCycle % m_kCoeffRegVsize;

                    coe0 = *coeff++;

                    xstart = (startDataOffset +
                              (offsetPhase + strobe * m_kPolyphaseLaneAlias) * m_kVOutSize * TP_DECIMATE_FACTOR /
                                  TP_INTERPOLATE_FACTOR) %
                             m_kSamplesInBuff; //

                    // dataNeeded += CEIL(TP_DECIMATE_FACTOR * m_kVOutSize,TP_INTERPOLATE_FACTOR) /
                    // TP_INTERPOLATE_FACTOR; // inaccurate due to CEIL()
                    //
                    if (streamInitNullMacPhase == streamInitNullAccs) {
#pragma unroll GUARD_ZERO((kMinDataLoadCycles))
                        for (int i = 0; i < kMinDataLoadCycles; i++) {
                            if (dataNeeded > dataLoaded) {
                                if (m_kStreamReadWidth == 256) {
                                    readStream256(sbuff, numDataLoads % m_kDataLoadsInReg, inInterface);
                                } else {
                                    readStream128(sbuff, numDataLoads % m_kDataLoadsInReg, inInterface,
                                                  inDataLoadPhase++ % 2);
                                }
                                dataLoaded += TP_INTERPOLATE_FACTOR * m_kStreamLoadVsize;
                                numDataLoads++;
                            }
                        }
                    } else {
#pragma unroll GUARD_ZERO((TP_DECIMATE_FACTOR))
                        for (int i = 0; i < TP_DECIMATE_FACTOR; i++) {
                            if (dataNeeded > dataLoaded) {
                                if (m_kStreamReadWidth == 256) {
                                    readStream256(sbuff, numDataLoads % m_kDataLoadsInReg, inInterface);
                                } else {
                                    readStream128(sbuff, numDataLoads % m_kDataLoadsInReg, inInterface,
                                                  inDataLoadPhase++ % 2);
                                }
                                dataLoaded += TP_INTERPOLATE_FACTOR * m_kStreamLoadVsize;
                                numDataLoads++;
                            }
                        }
                    }

                    acc = readCascade(inInterface, acc);

                    acc = initMacResampler(inInterface, acc, sbuff, xstart, xoffsets[offsetPhase], coe0,
                                           coeffPhaseStart, zoffsets[offsetPhase]);

                    dataNeeded += TP_DECIMATE_FACTOR * m_kVOutSize;

#pragma unroll(GUARD_ZERO((m_kNumOps - 1)))
                    // for (int op = 1; op < m_kNumOps; ++op) {
                    for (int op = m_kColumns; op < m_kFirLenCeilCols; op += m_kColumns) {
                        int coeffStart = (coeffPhaseStart + (op / m_kColumns) * m_kMultsPerCycle) % m_kCoeffRegVsize;

                        if (coeffStart == 0) {
                            coe0 = *coeff++;
                        }
                        acc = macResampler(acc, sbuff, (op + xstart), xoffsets[offsetPhase], coe0, coeffStart,
                                           zoffsets[offsetPhase]);
                    }

                    outVal = shiftAndSaturate(acc, TP_SHIFT);

                    // Write cascade. Do nothing if cascade not present.
                    writeCascade<TT_DATA, TT_COEFF>(outInterface, acc);

                    // writeOutput<TT_DATA, TT_COEFF, TP_NUM_OUTPUTS, TP_API>(outInterface, outVal, outDataPhase++%2);
                    writeStream<TT_DATA, TT_COEFF, TP_NUM_OUTPUTS>(outInterface, outVal, outDataPhase++ % 2);
                }

#pragma unroll(GUARD_ZERO(streamInitAccStrobes))
                for (unsigned strobe = CEIL(streamInitNullAccs, m_kPolyphaseLaneAlias) / m_kPolyphaseLaneAlias;
                     strobe < (m_kRepeatFactor); strobe++) {
// unroll up to interpolation factor times to get 0th polyphase is the first lane again.
#pragma unroll(m_kPolyphaseLaneAlias)
                    for (unsigned offsetPhase = 0; offsetPhase < m_kPolyphaseLaneAlias; ++offsetPhase) {
                        m_internalTapsCopy = chess_copy(m_internalTapsCopy);
                        coeff =
                            ((T_buff_256b<TT_COEFF>*)m_internalTapsCopy) +
                            (offsetPhase % m_kPolyphaseCoeffAlias) * m_kNumOps * m_kMultsPerCycle / m_kCoeffRegVsize;
                        int coeffPhaseStart =
                            (offsetPhase % m_kPolyphaseCoeffAlias) * m_kNumOps * m_kMultsPerCycle % m_kCoeffRegVsize;

                        coe0 = *coeff++;

                        xstart = (startDataOffset +
                                  (offsetPhase + strobe * m_kPolyphaseLaneAlias) * m_kVOutSize * TP_DECIMATE_FACTOR /
                                      TP_INTERPOLATE_FACTOR) %
                                 m_kSamplesInBuff; //
                        // dataNeeded += CEIL(TP_DECIMATE_FACTOR * m_kVOutSize,TP_INTERPOLATE_FACTOR) /
                        // TP_INTERPOLATE_FACTOR; // inaccurate due to CEIL()
                        if (offsetPhase == 0 && streamInitNullAccs >= CEIL(streamInitNullAccs, m_kPolyphaseLaneAlias) &&
                            strobe == CEIL(streamInitNullAccs, m_kPolyphaseLaneAlias) / m_kPolyphaseLaneAlias) {
// dataNeeded += TP_DECIMATE_FACTOR * m_kVOutSize ;
#pragma unroll GUARD_ZERO((kMinDataLoadCycles))
                            for (int i = 0; i < kMinDataLoadCycles; i++) {
                                if (dataNeeded > dataLoaded) {
                                    if (m_kStreamReadWidth == 256) {
                                        readStream256(sbuff, numDataLoads % m_kDataLoadsInReg, inInterface);
                                    } else {
                                        readStream128(sbuff, numDataLoads % m_kDataLoadsInReg, inInterface,
                                                      inDataLoadPhase++ % 2);
                                    }
                                    dataLoaded += TP_INTERPOLATE_FACTOR * m_kStreamLoadVsize;
                                    numDataLoads++;
                                }
                            }
                        } else {
#pragma unroll GUARD_ZERO((TP_DECIMATE_FACTOR))
                            for (int i = 0; i < TP_DECIMATE_FACTOR; i++) {
                                if (dataNeeded > dataLoaded) {
                                    if (m_kStreamReadWidth == 256) {
                                        readStream256(sbuff, numDataLoads % m_kDataLoadsInReg, inInterface);
                                    } else {
                                        readStream128(sbuff, numDataLoads % m_kDataLoadsInReg, inInterface,
                                                      inDataLoadPhase++ % 2);
                                    }
                                    dataLoaded += TP_INTERPOLATE_FACTOR * m_kStreamLoadVsize;
                                    numDataLoads++;
                                }
                            }
                        }

                        // Read cascade input. Do nothing if cascade input not present.
                        acc = readCascade(inInterface, acc);

                        // Init Vector operation. VMUL if cascade not present, otherwise VMAC
                        acc = initMacResampler(inInterface, acc, sbuff, xstart, xoffsets[offsetPhase], coe0,
                                               coeffPhaseStart, zoffsets[offsetPhase]);

                        dataNeeded += TP_DECIMATE_FACTOR * m_kVOutSize;

#pragma unroll(GUARD_ZERO((m_kNumOps - 1)))
                        for (int op = m_kColumns; op < m_kFirLenCeilCols; op += m_kColumns) {
                            int coeffStart =
                                (coeffPhaseStart + (op / m_kColumns) * m_kMultsPerCycle) % m_kCoeffRegVsize;

                            if (coeffStart == 0) {
                                coe0 = *coeff++;
                            }
                            acc = macResampler(acc, sbuff, (op + xstart), xoffsets[offsetPhase], coe0, coeffStart,
                                               zoffsets[offsetPhase]);
                        }

                        // Write cascade. Do nothing if cascade not present.
                        writeCascade<TT_DATA, TT_COEFF>(outInterface, acc);

                        outVal = shiftAndSaturate(acc, TP_SHIFT);

                        // Write to output window
                        // writeOutput<TT_DATA, TT_COEFF, TP_NUM_OUTPUTS, TP_API>(outInterface, outVal,
                        // outDataPhase++%2);
                        writeStream<TT_DATA, TT_COEFF, TP_NUM_OUTPUTS>(outInterface, outVal, outDataPhase++ % 2);
                    } // m_kPolyphaseLaneAlias
                }     // streamInitAccStrobes

                if
                    constexpr(streamInitNullAccs == 0) { loopSize -= 1; }
                else {
                    loopSize -= CEIL(streamInitNullAccs, m_kRepeatFactor) / m_kRepeatFactor;
                }
            }
        }
    doInit = 0;

    // This loop creates the output window data. In each iteration a vector of samples is output
    for (unsigned i = 0; i < (loopSize); i++)
        chess_prepare_for_pipelining chess_loop_range((m_kLsize / m_kRepeatFactor - 1), (m_kLsize / m_kRepeatFactor)) {
            numDataLoads = startDataLoads;
            dataLoaded = 0;
            dataNeeded = 0;

            inDataLoadPhase = numberLoadsInitPhase; //

// unroll up to m_kRepeatFactor times to get around xbuff in full.
#pragma unroll(m_kRepeatFactor)
            for (int strobe = 0; strobe < m_kRepeatFactor; strobe++) {
// unroll up to interpolation factor times to get 0th polyphase is the first lane again.
#pragma unroll(m_kPolyphaseLaneAlias)
                for (unsigned offsetPhase = 0; offsetPhase < m_kPolyphaseLaneAlias; ++offsetPhase) {
                    m_internalTapsCopy = chess_copy(m_internalTapsCopy);
                    // coeff = ((T_buff_256b<TT_COEFF> *)m_internalTapsCopy) +
                    // (offsetPhase%m_kPolyphaseCoeffAlias)*m_kNumOps;
                    // need to be able to cope with 128b alignment,
                    // as second component of the below may result in fractional number,
                    // e.g. 1 * 3 * 4 / 8 = 1.5.
                    // which gets truncated to int 1.
                    // Easy way to do this is to modify coeff register start point.
                    coeff = ((T_buff_256b<TT_COEFF>*)m_internalTapsCopy) +
                            (offsetPhase % m_kPolyphaseCoeffAlias) * m_kNumOps * m_kMultsPerCycle / m_kCoeffRegVsize;
                    int coeffPhaseStart =
                        (offsetPhase % m_kPolyphaseCoeffAlias) * m_kNumOps * m_kMultsPerCycle % m_kCoeffRegVsize;

                    coe0 = *coeff++;

                    xstart = (startDataOffset +
                              (offsetPhase + strobe * m_kPolyphaseLaneAlias) * m_kVOutSize * TP_DECIMATE_FACTOR /
                                  TP_INTERPOLATE_FACTOR) %
                             m_kSamplesInBuff; //

                    // dataNeeded += CEIL(TP_DECIMATE_FACTOR * m_kVOutSize,TP_INTERPOLATE_FACTOR) /
                    // TP_INTERPOLATE_FACTOR; // inaccurate due to CEIL()
                    dataNeeded += TP_DECIMATE_FACTOR * m_kVOutSize;
#pragma unroll GUARD_ZERO(TP_DECIMATE_FACTOR)
                    for (int i = 0; i < TP_DECIMATE_FACTOR; i++) {
                        if (dataNeeded > dataLoaded) {
                            if (m_kStreamReadWidth == 256) {
                                readStream256(sbuff, numDataLoads % m_kDataLoadsInReg, inInterface);
                            } else {
                                readStream128(sbuff, numDataLoads % m_kDataLoadsInReg, inInterface,
                                              inDataLoadPhase++ % 2);
                            }
                            dataLoaded += TP_INTERPOLATE_FACTOR * m_kStreamLoadVsize;
                            numDataLoads++;
                        }
                    }

                    // Read cascade input. Do nothing if cascade input not present.
                    acc = readCascade(inInterface, acc);

                    // Init Vector operation. VMUL if cascade not present, otherwise VMAC
                    acc = initMacResampler(inInterface, acc, sbuff, xstart, xoffsets[offsetPhase], coe0,
                                           coeffPhaseStart, zoffsets[offsetPhase]);

#pragma unroll(GUARD_ZERO((m_kNumOps - 1)))
                    for (int op = m_kColumns; op < m_kFirLenCeilCols; op += m_kColumns) {
                        int coeffStart = (coeffPhaseStart + (op / m_kColumns) * m_kMultsPerCycle) % m_kCoeffRegVsize;

                        if (coeffStart == 0) {
                            coe0 = *coeff++;
                        }
                        acc = macResampler(acc, sbuff, (op + xstart), xoffsets[offsetPhase], coe0, coeffStart,
                                           zoffsets[offsetPhase]);
                    }

                    // Write cascade. Do nothing if cascade not present.
                    writeCascade<TT_DATA, TT_COEFF>(outInterface, acc);

                    outVal = shiftAndSaturate(acc, TP_SHIFT);

                    // Write to output window
                    // writeOutput<TT_DATA, TT_COEFF, TP_NUM_OUTPUTS, TP_API>(outInterface, outVal, outDataPhase++%2);
                    writeStream<TT_DATA, TT_COEFF, TP_NUM_OUTPUTS>(outInterface, outVal, outDataPhase++ % 2);
                } // m_kPolyphaseLaneAlias
            }     // m_kRepeatFactor
        }         // LSize

    doInit = 0;

    // store sbuff for next iteration
    *ptr_delay = sbuff;
};

// Single kernel base specialization. Windowed. No cascade ports. Static coefficients
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          bool TP_CASC_IN,
          bool TP_CASC_OUT,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN,
          unsigned int TP_USE_COEFF_RELOAD,
          unsigned int TP_NUM_OUTPUTS,
          unsigned int TP_DUAL_IP,
          unsigned int TP_API>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   TP_CASC_IN,
                   TP_CASC_OUT,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   TP_USE_COEFF_RELOAD,
                   TP_NUM_OUTPUTS,
                   TP_DUAL_IP,
                   TP_API>::
    filter(input_circular_buffer<TT_DATA,
                                 extents<inherited_extent>,
                                 margin<fnFirMargin<(TP_FIR_LEN + TP_INTERPOLATE_FACTOR - 1) / TP_INTERPOLATE_FACTOR,
                                                    TT_DATA>()> >& __restrict inWindow,
           output_circular_buffer<TT_DATA>& __restrict outWindow) {
    T_inputIF<CASC_IN_FALSE, TT_DATA, TP_DUAL_IP> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inWindowCirc = (input_circular_buffer<TT_DATA, extents<inherited_extent>, margin<16> >*)&inWindow;
    outInterface.outWindow = (output_circular_buffer<TT_DATA>*)&outWindow;
    this->filterKernel(inInterface, outInterface);
};

// Single kernel specialization. Windowed. No cascade ports. Static coefficients, dual output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_FALSE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   0,
                   1,
                   USE_COEFF_RELOAD_FALSE,
                   2,
                   DUAL_IP_SINGLE,
                   USE_WINDOW_API>::
    filter(input_circular_buffer<TT_DATA,
                                 extents<inherited_extent>,
                                 margin<fnFirMargin<(TP_FIR_LEN + TP_INTERPOLATE_FACTOR - 1) / TP_INTERPOLATE_FACTOR,
                                                    TT_DATA>()> >& __restrict inWindow,
           output_circular_buffer<TT_DATA>& __restrict outWindow,
           output_circular_buffer<TT_DATA>& __restrict outWindow2) {
    T_inputIF<CASC_IN_FALSE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inWindowCirc = (input_circular_buffer<TT_DATA, extents<inherited_extent>, margin<16> >*)&inWindow;
    outInterface.outWindow = (output_circular_buffer<TT_DATA>*)&outWindow;
    outInterface.outWindow2 = (output_circular_buffer<TT_DATA>*)&outWindow2;
    this->filterKernel(inInterface, outInterface);
};

// Single kernel specialization. Windowed. No cascade ports, with reload coefficients, single output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_FALSE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   1,
                   DUAL_IP_SINGLE,
                   USE_WINDOW_API>::
    filter(input_circular_buffer<TT_DATA,
                                 extents<inherited_extent>,
                                 margin<fnFirMargin<(TP_FIR_LEN + TP_INTERPOLATE_FACTOR - 1) / TP_INTERPOLATE_FACTOR,
                                                    TT_DATA>()> >& __restrict inWindow,
           output_circular_buffer<TT_DATA>& __restrict outWindow,
           const TT_COEFF (&inTaps)[TP_FIR_LEN]) {
    T_inputIF<CASC_IN_FALSE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inWindowCirc = (input_circular_buffer<TT_DATA, extents<inherited_extent>, margin<16> >*)&inWindow;
    outInterface.outWindow = (output_circular_buffer<TT_DATA>*)&outWindow;
    this->filterKernel(inInterface, outInterface, inTaps);
};

// Single kernel specialization. No cascade ports, Windowed. with reload coefficients, dual output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_FALSE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   2,
                   DUAL_IP_SINGLE,
                   USE_WINDOW_API>::
    filter(input_circular_buffer<TT_DATA,
                                 extents<inherited_extent>,
                                 margin<fnFirMargin<(TP_FIR_LEN + TP_INTERPOLATE_FACTOR - 1) / TP_INTERPOLATE_FACTOR,
                                                    TT_DATA>()> >& __restrict inWindow,
           output_circular_buffer<TT_DATA>& __restrict outWindow,
           output_circular_buffer<TT_DATA>& __restrict outWindow2,
           const TT_COEFF (&inTaps)[TP_FIR_LEN]) {
    T_inputIF<CASC_IN_FALSE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inWindowCirc = (input_circular_buffer<TT_DATA, extents<inherited_extent>, margin<16> >*)&inWindow;
    outInterface.outWindow = (output_circular_buffer<TT_DATA>*)&outWindow;
    outInterface.outWindow2 = (output_circular_buffer<TT_DATA>*)&outWindow2;
    this->filterKernel(inInterface, outInterface, inTaps);
};

// Partially specialized classes for cascaded interface (final kernel in cascade), Windowed. no reload, single output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_FALSE,
                   1,
                   DUAL_IP_SINGLE,
                   USE_WINDOW_API>::filter(input_async_buffer<TT_DATA, extents<inherited_extent> >& inWindow,
                                           input_stream_cacc48* inCascade,
                                           output_circular_buffer<TT_DATA>& __restrict outWindow) {
    T_inputIF<CASC_IN_TRUE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inWindowLin = (input_async_buffer<TT_DATA>*)&inWindow;
    inInterface.inCascade = inCascade;
    outInterface.outWindow = (output_circular_buffer<TT_DATA>*)&outWindow;
    this->filterKernel(inInterface, outInterface);
};

// Partially specialized classes for cascaded interface (final kernel in cascade), Windowed. no reload, dual output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_FALSE,
                   2,
                   DUAL_IP_SINGLE,
                   USE_WINDOW_API>::filter(input_async_buffer<TT_DATA, extents<inherited_extent> >& inWindow,
                                           input_stream_cacc48* inCascade,
                                           output_circular_buffer<TT_DATA>& __restrict outWindow,
                                           output_circular_buffer<TT_DATA>& __restrict outWindow2) {
    T_inputIF<CASC_IN_TRUE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inWindowLin = (input_async_buffer<TT_DATA>*)&inWindow;
    inInterface.inCascade = inCascade;
    outInterface.outWindow = (output_circular_buffer<TT_DATA>*)&outWindow;
    outInterface.outWindow2 = (output_circular_buffer<TT_DATA>*)&outWindow2;
    this->filterKernel(inInterface, outInterface);
};

// Partially specialized classes for cascaded interface (final kernel in cascade), Windowed. with reload, single output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   1,
                   DUAL_IP_SINGLE,
                   USE_WINDOW_API>::filter(input_async_buffer<TT_DATA, extents<inherited_extent> >& inWindow,
                                           input_stream_cacc48* inCascade,
                                           output_circular_buffer<TT_DATA>& __restrict outWindow) {
    T_inputIF<CASC_IN_TRUE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inWindowLin = (input_async_buffer<TT_DATA>*)&inWindow;
    inInterface.inCascade = inCascade;
    outInterface.outWindow = (output_circular_buffer<TT_DATA>*)&outWindow;
    this->filterKernelRtp(inInterface, outInterface);
};

// Partially specialized classes for cascaded interface - final kernel. Reloadable coefficients, dual output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   2,
                   DUAL_IP_SINGLE,
                   USE_WINDOW_API>::filter(input_async_buffer<TT_DATA, extents<inherited_extent> >& inWindow,
                                           input_stream_cacc48* inCascade,
                                           output_circular_buffer<TT_DATA>& __restrict outWindow,
                                           output_circular_buffer<TT_DATA>& __restrict outWindow2) {
    T_inputIF<CASC_IN_TRUE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inWindowLin = (input_async_buffer<TT_DATA>*)&inWindow;
    inInterface.inCascade = inCascade;
    outInterface.outWindow = (output_circular_buffer<TT_DATA>*)&outWindow;
    outInterface.outWindow2 = (output_circular_buffer<TT_DATA>*)&outWindow2;
    this->filterKernelRtp(inInterface, outInterface);
};

// Partially specialized classes for cascaded interface (first kernel in cascade), Windowed. no reload, single output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_FALSE,
                   CASC_OUT_TRUE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_FALSE,
                   1,
                   DUAL_IP_SINGLE,
                   USE_WINDOW_API>::
    filter(input_circular_buffer<
               TT_DATA,
               extents<inherited_extent>,
               margin<fnFirMargin<(TP_FIR_LEN + TP_INTERPOLATE_FACTOR - 1) / TP_INTERPOLATE_FACTOR, TT_DATA>()> >&
               inWindow,
           output_stream_cacc48* outCascade,
           output_async_buffer<TT_DATA>& broadcastWindow) {
    T_inputIF<CASC_IN_FALSE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_TRUE, TT_DATA> outInterface;
    inInterface.inWindowCirc = (input_circular_buffer<TT_DATA, extents<inherited_extent>, margin<16> >*)&inWindow;
    outInterface.outCascade = outCascade;
    outInterface.broadcastWindow = (output_async_buffer<TT_DATA>*)&broadcastWindow;
    outInterface.outWindow = (input_circular_buffer<TT_DATA, extents<inherited_extent>,
                                                    margin<16> >*)&inWindow; // dummy to give outItr a hook
    this->filterKernel(inInterface, outInterface);
};

// Partially specialized classes for cascaded interface (First kernel in cascade), Windowed. with reload
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_FALSE,
                   CASC_OUT_TRUE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   1,
                   DUAL_IP_SINGLE,
                   USE_WINDOW_API>::
    filter(input_circular_buffer<
               TT_DATA,
               extents<inherited_extent>,
               margin<fnFirMargin<(TP_FIR_LEN + TP_INTERPOLATE_FACTOR - 1) / TP_INTERPOLATE_FACTOR, TT_DATA>()> >&
               inWindow,
           output_stream_cacc48* outCascade,
           output_async_buffer<TT_DATA>& broadcastWindow,
           const TT_COEFF (&inTaps)[TP_FIR_LEN]) {
    T_inputIF<CASC_IN_FALSE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_TRUE, TT_DATA> outInterface;
    inInterface.inWindowCirc = (input_circular_buffer<TT_DATA, extents<inherited_extent>, margin<16> >*)&inWindow;
    outInterface.outCascade = outCascade;
    outInterface.broadcastWindow = (output_async_buffer<TT_DATA>*)&broadcastWindow;
    outInterface.outWindow = (input_circular_buffer<TT_DATA, extents<inherited_extent>,
                                                    margin<16> >*)&inWindow; // dummy to give outItr a hook
    this->filterKernel(inInterface, outInterface, inTaps);
};

// Partially specialized classes for cascaded interface (middle kernels in cascade), Windowed. no reload
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_TRUE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_FALSE,
                   1,
                   DUAL_IP_SINGLE,
                   USE_WINDOW_API>::filter(input_async_buffer<TT_DATA>& inWindow,
                                           input_stream_cacc48* inCascade,
                                           output_stream_cacc48* outCascade,
                                           output_async_buffer<TT_DATA>& broadcastWindow) {
    T_inputIF<CASC_IN_TRUE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_TRUE, TT_DATA> outInterface;
    inInterface.inWindowLin = (input_async_buffer<TT_DATA>*)&inWindow;
    inInterface.inCascade = inCascade;
    outInterface.outCascade = outCascade;
    outInterface.broadcastWindow = (output_async_buffer<TT_DATA>*)&broadcastWindow;
    outInterface.outWindow = (input_circular_buffer<TT_DATA, extents<inherited_extent>,
                                                    margin<16> >*)&inWindow; // dummy to give outItr a hook
    this->filterKernel(inInterface, outInterface);
};

// Partially specialized classes for cascaded interface (middle kernels in cascade), Windowed. with reload
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_TRUE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   1,
                   DUAL_IP_SINGLE,
                   USE_WINDOW_API>::filter(input_async_buffer<TT_DATA>& inWindow,
                                           input_stream_cacc48* inCascade,
                                           output_stream_cacc48* outCascade,
                                           output_async_buffer<TT_DATA>& broadcastWindow) {
    T_inputIF<CASC_IN_TRUE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_TRUE, TT_DATA> outInterface;
    inInterface.inWindowLin = (input_async_buffer<TT_DATA>*)&inWindow;
    inInterface.inCascade = inCascade;
    outInterface.outCascade = outCascade;
    outInterface.broadcastWindow = (output_async_buffer<TT_DATA>*)&broadcastWindow;
    outInterface.outWindow = (input_circular_buffer<TT_DATA, extents<inherited_extent>,
                                                    margin<16> >*)&inWindow; // dummy to give outItr a hook
    this->filterKernelRtp(inInterface, outInterface);
};

// ----------------------------------------------------------------------------
// ---------------------------------- STREAM ----------------------------------
// ----------------------------------------------------------------------------

// Single kernel specialization. No cascade ports. Streaming. Static coefficients, single input, single output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_FALSE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   0,
                   1,
                   USE_COEFF_RELOAD_FALSE,
                   1,
                   DUAL_IP_SINGLE,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream, output_stream<TT_DATA>* outStream) {
    T_inputIF<CASC_IN_FALSE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    outInterface.outStream = outStream;
    this->filterKernel(inInterface, outInterface);
};

// Single kernel, Static coefficients, dual output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_FALSE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   0,
                   1,
                   USE_COEFF_RELOAD_FALSE,
                   2,
                   DUAL_IP_SINGLE,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           output_stream<TT_DATA>* outStream,
                                           output_stream<TT_DATA>* outStream2) {
    T_inputIF<CASC_IN_FALSE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    outInterface.outStream = outStream;
    outInterface.outStream2 = outStream2;
    this->filterKernel(inInterface, outInterface);
};

// This is a specialization of the main class for when there is only one kernel for the whole filter.
// Reloadable coefficients, single output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_FALSE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   1,
                   DUAL_IP_SINGLE,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           output_stream<TT_DATA>* outStream,
                                           const TT_COEFF (&inTaps)[TP_FIR_LEN]) {
    T_inputIF<CASC_IN_FALSE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    outInterface.outStream = outStream;
    this->filterKernel(inInterface, outInterface, inTaps);
};

// This is a specialization of the main class for when there is only one kernel for the whole filter.
// Reloadable coefficients, dual output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_FALSE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   2,
                   DUAL_IP_SINGLE,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           output_stream<TT_DATA>* outStream,
                                           output_stream<TT_DATA>* outStream2,
                                           const TT_COEFF (&inTaps)[TP_FIR_LEN]) {
    T_inputIF<CASC_IN_FALSE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    outInterface.outStream = outStream;
    outInterface.outStream2 = outStream2;
    this->filterKernel(inInterface, outInterface, inTaps);
};

// This is a specialization of the main class for the final kernel in a cascade chain.
// Static coefficients, single output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_FALSE,
                   1,
                   DUAL_IP_SINGLE,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream_cacc48* inCascade,
                                           output_stream<TT_DATA>* outStream) {
    T_inputIF<CASC_IN_TRUE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inCascade = inCascade;
    outInterface.outStream = outStream;
    this->filterKernel(inInterface, outInterface);
};

// This is a specialization of the main class for the final kernel in a cascade chain.
// Static coefficients, dual output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_FALSE,
                   2,
                   DUAL_IP_SINGLE,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream_cacc48* inCascade,
                                           output_stream<TT_DATA>* outStream,
                                           output_stream<TT_DATA>* outStream2) {
    T_inputIF<CASC_IN_TRUE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inCascade = inCascade;
    outInterface.outStream = outStream;
    outInterface.outStream2 = outStream2;
    this->filterKernel(inInterface, outInterface);
};

// This is a specialization of the main class for the first kernel in a cascade chain.
// Static coefficients
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_FALSE,
                   CASC_OUT_TRUE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_FALSE,
                   1,
                   DUAL_IP_SINGLE,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream, output_stream_cacc48* outCascade) {
    T_inputIF<CASC_IN_FALSE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_TRUE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    outInterface.outCascade = outCascade;
    this->filterKernel(inInterface, outInterface);
};

// This is a specialization of the main class for any kernel within a cascade chain, but neither first nor last.
// Static coefficients
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_TRUE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_FALSE,
                   1,
                   DUAL_IP_SINGLE,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream_cacc48* inCascade,
                                           output_stream_cacc48* outCascade) {
    T_inputIF<CASC_IN_TRUE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_TRUE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inCascade = inCascade;
    outInterface.outCascade = outCascade;
    this->filterKernel(inInterface, outInterface);
};

// This is a specialization of the main class for the final kernel in a cascade chain.
// Reloadable coefficients, single output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   1,
                   DUAL_IP_SINGLE,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream_cacc48* inCascade,
                                           output_stream<TT_DATA>* outStream) {
    T_inputIF<CASC_IN_TRUE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inCascade = inCascade;
    outInterface.outStream = outStream;
    this->filterKernelRtp(inInterface, outInterface);
};

// This is a specialization of the main class for the final kernel in a cascade chain.
// Reloadable coefficients, dual output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   2,
                   DUAL_IP_SINGLE,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream_cacc48* inCascade,
                                           output_stream<TT_DATA>* outStream,
                                           output_stream<TT_DATA>* outStream2) {
    T_inputIF<CASC_IN_TRUE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inCascade = inCascade;
    outInterface.outStream = outStream;
    outInterface.outStream2 = outStream2;
    this->filterKernelRtp(inInterface, outInterface);
};

// This is a specialization of the main class for the first kernel in a cascade chain.
// Reloadable coefficients
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_FALSE,
                   CASC_OUT_TRUE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   1,
                   DUAL_IP_SINGLE,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           output_stream_cacc48* outCascade,
                                           const TT_COEFF (&inTaps)[TP_FIR_LEN]) {
    T_inputIF<CASC_IN_FALSE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_TRUE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    outInterface.outCascade = outCascade;
    this->filterKernel(inInterface, outInterface, inTaps);
};

// This is a specialization of the main class for any kernel within a cascade chain, but neither first nor last.
// Reloadable coefficients
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_TRUE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   1,
                   DUAL_IP_SINGLE,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream_cacc48* inCascade,
                                           output_stream_cacc48* outCascade) {
    T_inputIF<CASC_IN_TRUE, TT_DATA> inInterface;
    T_outputIF<CASC_OUT_TRUE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inCascade = inCascade;
    outInterface.outCascade = outCascade;
    this->filterKernelRtp(inInterface, outInterface);
};

// ----------------------------------------------------------------------------
// ----------------------------- DUAL STREAM ----------------------------------
// ----------------------------------------------------------------------------

// Single kernel, Static coefficients, dual input
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_FALSE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   0,
                   1,
                   USE_COEFF_RELOAD_FALSE,
                   1,
                   DUAL_IP_DUAL,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream<TT_DATA>* inStream2,
                                           output_stream<TT_DATA>* outStream) {
    T_inputIF<CASC_IN_FALSE, TT_DATA, DUAL_IP_DUAL> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inStream2 = inStream2;
    outInterface.outStream = outStream;
    this->filterKernel(inInterface, outInterface);
};

// Single kernel, Static coefficients, dual output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_FALSE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   0,
                   1,
                   USE_COEFF_RELOAD_FALSE,
                   2,
                   DUAL_IP_DUAL,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream<TT_DATA>* inStream2,
                                           output_stream<TT_DATA>* outStream,
                                           output_stream<TT_DATA>* outStream2) {
    T_inputIF<CASC_IN_FALSE, TT_DATA, DUAL_IP_DUAL> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inStream2 = inStream2;
    outInterface.outStream = outStream;
    outInterface.outStream2 = outStream2;
    this->filterKernel(inInterface, outInterface);
};

// This is a specialization of the main class for when there is only one kernel for the whole filter.
// Reloadable coefficients, dual input
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_FALSE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   1,
                   DUAL_IP_DUAL,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream<TT_DATA>* inStream2,
                                           output_stream<TT_DATA>* outStream,
                                           const TT_COEFF (&inTaps)[TP_FIR_LEN]) {
    T_inputIF<CASC_IN_FALSE, TT_DATA, DUAL_IP_DUAL> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inStream2 = inStream2;
    outInterface.outStream = outStream;
    this->filterKernel(inInterface, outInterface, inTaps);
};

// This is a specialization of the main class for when there is only one kernel for the whole filter.
// Reloadable coefficients, dual output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_FALSE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   2,
                   DUAL_IP_DUAL,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream<TT_DATA>* inStream2,
                                           output_stream<TT_DATA>* outStream,
                                           output_stream<TT_DATA>* outStream2,
                                           const TT_COEFF (&inTaps)[TP_FIR_LEN]) {
    T_inputIF<CASC_IN_FALSE, TT_DATA, DUAL_IP_DUAL> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inStream2 = inStream2;
    outInterface.outStream = outStream;
    outInterface.outStream2 = outStream2;
    this->filterKernel(inInterface, outInterface, inTaps);
};

// This is a specialization of the main class for the first kernel in a cascade chain.
// Static coefficients
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_FALSE,
                   CASC_OUT_TRUE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_FALSE,
                   1,
                   DUAL_IP_DUAL,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream<TT_DATA>* inStream2,
                                           output_stream_cacc48* outCascade) {
    T_inputIF<CASC_IN_FALSE, TT_DATA, DUAL_IP_DUAL> inInterface;
    T_outputIF<CASC_OUT_TRUE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inStream2 = inStream2;
    outInterface.outCascade = outCascade;
    this->filterKernel(inInterface, outInterface);
};

// This is a specialization of the main class for any kernel within a cascade chain, but neither first nor last.
// Static coefficients
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_TRUE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_FALSE,
                   1,
                   DUAL_IP_DUAL,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream<TT_DATA>* inStream2,
                                           input_stream_cacc48* inCascade,
                                           output_stream_cacc48* outCascade) {
    T_inputIF<CASC_IN_TRUE, TT_DATA, DUAL_IP_DUAL> inInterface;
    T_outputIF<CASC_OUT_TRUE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inStream2 = inStream2;
    inInterface.inCascade = inCascade;
    outInterface.outCascade = outCascade;
    this->filterKernel(inInterface, outInterface);
};

// This is a specialization of the main class for the final kernel in a cascade chain.
// Static coefficients, dual output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_FALSE,
                   1,
                   DUAL_IP_DUAL,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream<TT_DATA>* inStream2,
                                           input_stream_cacc48* inCascade,
                                           output_stream<TT_DATA>* outStream) {
    T_inputIF<CASC_IN_TRUE, TT_DATA, DUAL_IP_DUAL> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inStream2 = inStream2;
    inInterface.inCascade = inCascade;
    outInterface.outStream = outStream;
    this->filterKernel(inInterface, outInterface);
};
// This is a specialization of the main class for the final kernel in a cascade chain.
// Static coefficients, dual output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_FALSE,
                   2,
                   DUAL_IP_DUAL,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream<TT_DATA>* inStream2,
                                           input_stream_cacc48* inCascade,
                                           output_stream<TT_DATA>* outStream,
                                           output_stream<TT_DATA>* outStream2) {
    T_inputIF<CASC_IN_TRUE, TT_DATA, DUAL_IP_DUAL> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inStream2 = inStream2;
    inInterface.inCascade = inCascade;
    outInterface.outStream = outStream;
    outInterface.outStream2 = outStream2;
    this->filterKernel(inInterface, outInterface);
};

// This is a specialization of the main class for the final kernel in a cascade chain.
// Reloadable coefficients, dual output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   2,
                   DUAL_IP_DUAL,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream<TT_DATA>* inStream2,
                                           input_stream_cacc48* inCascade,
                                           output_stream<TT_DATA>* outStream,
                                           output_stream<TT_DATA>* outStream2) {
    T_inputIF<CASC_IN_TRUE, TT_DATA, DUAL_IP_DUAL> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inStream2 = inStream2;
    inInterface.inCascade = inCascade;
    outInterface.outStream = outStream;
    outInterface.outStream2 = outStream2;
    this->filterKernelRtp(inInterface, outInterface);
};

// This is a specialization of the main class for the final kernel in a cascade chain.
// Reloadable coefficients, dual output
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_FALSE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   1,
                   DUAL_IP_DUAL,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream<TT_DATA>* inStream2,
                                           input_stream_cacc48* inCascade,
                                           output_stream<TT_DATA>* outStream) {
    T_inputIF<CASC_IN_TRUE, TT_DATA, DUAL_IP_DUAL> inInterface;
    T_outputIF<CASC_OUT_FALSE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inStream2 = inStream2;
    inInterface.inCascade = inCascade;
    outInterface.outStream = outStream;
    this->filterKernelRtp(inInterface, outInterface);
};

// This is a specialization of the main class for the first kernel in a cascade chain.
// Reloadable coefficients
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_FALSE,
                   CASC_OUT_TRUE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   1,
                   DUAL_IP_DUAL,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream<TT_DATA>* inStream2,
                                           output_stream_cacc48* outCascade,
                                           const TT_COEFF (&inTaps)[TP_FIR_LEN]) {
    T_inputIF<CASC_IN_FALSE, TT_DATA, DUAL_IP_DUAL> inInterface;
    T_outputIF<CASC_OUT_TRUE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inStream2 = inStream2;
    outInterface.outCascade = outCascade;
    this->filterKernel(inInterface, outInterface, inTaps);
};

// This is a specialization of the main class for any kernel within a cascade chain, but neither first nor last.
// Reloadable coefficients
//-----------------------------------------------------------------------------------------------------
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_INTERPOLATE_FACTOR,
          unsigned int TP_DECIMATE_FACTOR,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_FIR_RANGE_LEN,
          unsigned int TP_KERNEL_POSITION,
          unsigned int TP_CASC_LEN>
void fir_resampler<TT_DATA,
                   TT_COEFF,
                   TP_FIR_LEN,
                   TP_INTERPOLATE_FACTOR,
                   TP_DECIMATE_FACTOR,
                   TP_SHIFT,
                   TP_RND,
                   TP_INPUT_WINDOW_VSIZE,
                   CASC_IN_TRUE,
                   CASC_OUT_TRUE,
                   TP_FIR_RANGE_LEN,
                   TP_KERNEL_POSITION,
                   TP_CASC_LEN,
                   USE_COEFF_RELOAD_TRUE,
                   1,
                   DUAL_IP_DUAL,
                   USE_STREAM_API>::filter(input_stream<TT_DATA>* inStream,
                                           input_stream<TT_DATA>* inStream2,
                                           input_stream_cacc48* inCascade,
                                           output_stream_cacc48* outCascade) {
    T_inputIF<CASC_IN_TRUE, TT_DATA, DUAL_IP_DUAL> inInterface;
    T_outputIF<CASC_OUT_TRUE, TT_DATA> outInterface;
    inInterface.inStream = inStream;
    inInterface.inStream2 = inStream2;
    inInterface.inCascade = inCascade;
    outInterface.outCascade = outCascade;
    this->filterKernelRtp(inInterface, outInterface);
};
}
}
}
}
}
