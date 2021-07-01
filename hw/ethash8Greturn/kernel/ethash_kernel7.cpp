/*
 * Copyright 2019 Xilinx, Inc.
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

#include "ethash_kernel.hpp"
#include "ethashV8return.hpp"
#ifndef __SYNTHESIS__
#include <iostream>
#endif

inline void writeOut7(ap_uint<32>* result, ap_uint<32>* ret) {
    for (int i = 0; i < 17; i++) {
#pragma HLS_PIPELINE II = 1
        ret[i + 119] = result[i];
    }
}

extern "C" void ethash_kernel7(unsigned full_size,
                               unsigned header_hash0,
                               unsigned header_hash1,
                               unsigned header_hash2,
                               unsigned header_hash3,
                               unsigned header_hash4,
                               unsigned header_hash5,
                               unsigned header_hash6,
                               unsigned header_hash7,
                               unsigned boundary0,
                               unsigned boundary1,
                               unsigned boundary2,
                               unsigned boundary3,
                               unsigned boundary4,
                               unsigned boundary5,
                               unsigned boundary6,
                               unsigned boundary7,

                               unsigned nonce_start0,
                               unsigned nonce_start1,

                               unsigned batch_cnt,

                               ap_uint<512>* dram0,
                               ap_uint<512>* dram1,
                               ap_uint<512>* dram2,
                               ap_uint<512>* dram3,

                               ap_uint<32>* ret) {
// clang-format off

#pragma HLS INTERFACE m_axi offset = slave latency = 64 \
    num_write_outstanding = 1 num_read_outstanding = 128 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_0 port = dram0

#pragma HLS INTERFACE m_axi offset = slave latency = 64 \
    num_write_outstanding = 1 num_read_outstanding = 128 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_1 port = dram1

#pragma HLS INTERFACE m_axi offset = slave latency = 64 \
    num_write_outstanding = 1 num_read_outstanding = 128 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_2 port = dram2

#pragma HLS INTERFACE m_axi offset = slave latency = 64 \
    num_write_outstanding = 1 num_read_outstanding = 128 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_3 port = dram3

#pragma HLS INTERFACE m_axi offset = slave latency = 64 \
    num_write_outstanding = 1 num_read_outstanding = 128 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_3 port = ret

#pragma HLS INTERFACE s_axilite port = full_size bundle = control
#pragma HLS INTERFACE s_axilite port = header_hash0 bundle = control
#pragma HLS INTERFACE s_axilite port = header_hash1 bundle = control
#pragma HLS INTERFACE s_axilite port = header_hash2 bundle = control
#pragma HLS INTERFACE s_axilite port = header_hash3 bundle = control
#pragma HLS INTERFACE s_axilite port = header_hash4 bundle = control
#pragma HLS INTERFACE s_axilite port = header_hash5 bundle = control
#pragma HLS INTERFACE s_axilite port = header_hash6 bundle = control
#pragma HLS INTERFACE s_axilite port = header_hash7 bundle = control
#pragma HLS INTERFACE s_axilite port = boundary0 bundle = control
#pragma HLS INTERFACE s_axilite port = boundary1 bundle = control
#pragma HLS INTERFACE s_axilite port = boundary2 bundle = control
#pragma HLS INTERFACE s_axilite port = boundary3 bundle = control
#pragma HLS INTERFACE s_axilite port = boundary4 bundle = control
#pragma HLS INTERFACE s_axilite port = boundary5 bundle = control
#pragma HLS INTERFACE s_axilite port = boundary6 bundle = control
#pragma HLS INTERFACE s_axilite port = boundary7 bundle = control
#pragma HLS INTERFACE s_axilite port = nonce_start0 bundle = control
#pragma HLS INTERFACE s_axilite port = nonce_start1 bundle = control
#pragma HLS INTERFACE s_axilite port = batch_cnt bundle = control
#pragma HLS INTERFACE s_axilite port = dram0 bundle = control
#pragma HLS INTERFACE s_axilite port = dram1 bundle = control
#pragma HLS INTERFACE s_axilite port = dram2 bundle = control
#pragma HLS INTERFACE s_axilite port = dram3 bundle = control
#pragma HLS INTERFACE s_axilite port = ret bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control

// clang-format on

#ifndef __SYNTHESIS__
    std::cout << "kernel start" << std::endl;
#endif

    ap_uint<32> resultOut[17];
#pragma HLS resource variable = resultOut core = RAM_1P_LUTRAM

    ethashcore(full_size, header_hash0, header_hash1, header_hash2, header_hash3, header_hash4, header_hash5,
               header_hash6, header_hash7, boundary0, boundary1, boundary2, boundary3, boundary4, boundary5, boundary6,
               boundary7, nonce_start0, nonce_start1, batch_cnt, dram0, dram1, dram2, dram3, resultOut);

    writeOut7(resultOut, ret); // Each kernel write to different location

#ifndef __SYNTHESIS__
    std::cout << "kernel end" << std::endl;
#endif
}
