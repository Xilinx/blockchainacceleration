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

#include "dag_kernel.hpp"
#ifndef __SYNTHESIS__
#include <iostream>
#endif

void wrapper(ap_uint<512>* cache_nodes_0,
             ap_uint<512>* cache_nodes_1,
             ap_uint<512>* cache_nodes_2,
             ap_uint<512>* cache_nodes_3,
             ap_uint<512>* cache_nodes_4,
             ap_uint<512>* cache_nodes_5,
             ap_uint<512>* cache_nodes_6,
             ap_uint<512>* cache_nodes_7,
             ap_uint<512>* cache_nodes_8,
             ap_uint<512>* cache_nodes_9,
             ap_uint<512>* cache_nodes_10,
             ap_uint<512>* cache_nodes_11,
             ap_uint<512>* cache_nodes_12,
             ap_uint<512>* cache_nodes_13,
             ap_uint<512>* cache_nodes_14,
             ap_uint<512>* cache_nodes_15,
             ap_uint<512>* dag_nodes_0,
             ap_uint<512>* dag_nodes_1,
             ap_uint<512>* dag_nodes_2,
             ap_uint<512>* dag_nodes_3,
             ap_uint<512>* dag_nodes_4,
             ap_uint<512>* dag_nodes_5,
             ap_uint<512>* dag_nodes_6,
             ap_uint<512>* dag_nodes_7,
             ap_uint<512>* dag_nodes_8,
             ap_uint<512>* dag_nodes_9,
             ap_uint<512>* dag_nodes_10,
             ap_uint<512>* dag_nodes_11,
             ap_uint<512>* dag_nodes_12,
             ap_uint<512>* dag_nodes_13,
             ap_uint<512>* dag_nodes_14,
             ap_uint<512>* dag_nodes_15,
             ap_uint<32> cache_nodes_num,
             ap_uint<32> dag_bias[16],
             ap_uint<32> dag_node_count) {
#pragma HLS dataflow

    dag_gen<12>(cache_nodes_0, cache_nodes_num, dag_nodes_0, dag_bias[0], dag_node_count);
    dag_gen<12>(cache_nodes_1, cache_nodes_num, dag_nodes_1, dag_bias[1], dag_node_count);
    dag_gen<12>(cache_nodes_2, cache_nodes_num, dag_nodes_2, dag_bias[2], dag_node_count);
    dag_gen<12>(cache_nodes_3, cache_nodes_num, dag_nodes_3, dag_bias[3], dag_node_count);
    dag_gen<12>(cache_nodes_4, cache_nodes_num, dag_nodes_4, dag_bias[4], dag_node_count);
    dag_gen<12>(cache_nodes_5, cache_nodes_num, dag_nodes_5, dag_bias[5], dag_node_count);
    dag_gen<12>(cache_nodes_6, cache_nodes_num, dag_nodes_6, dag_bias[6], dag_node_count);
    dag_gen<12>(cache_nodes_7, cache_nodes_num, dag_nodes_7, dag_bias[7], dag_node_count);
    dag_gen<12>(cache_nodes_8, cache_nodes_num, dag_nodes_8, dag_bias[8], dag_node_count);
    dag_gen<12>(cache_nodes_9, cache_nodes_num, dag_nodes_9, dag_bias[9], dag_node_count);
    dag_gen<12>(cache_nodes_10, cache_nodes_num, dag_nodes_10, dag_bias[10], dag_node_count);
    dag_gen<12>(cache_nodes_11, cache_nodes_num, dag_nodes_11, dag_bias[11], dag_node_count);
    dag_gen<12>(cache_nodes_12, cache_nodes_num, dag_nodes_12, dag_bias[12], dag_node_count);
    dag_gen<12>(cache_nodes_13, cache_nodes_num, dag_nodes_13, dag_bias[13], dag_node_count);
    dag_gen<12>(cache_nodes_14, cache_nodes_num, dag_nodes_14, dag_bias[14], dag_node_count);
    dag_gen<12>(cache_nodes_15, cache_nodes_num, dag_nodes_15, dag_bias[15], dag_node_count);
}

extern "C" void dag_gen_kernel(ap_uint<512>* cache_nodes_0,
                               ap_uint<512>* cache_nodes_1,
                               ap_uint<512>* cache_nodes_2,
                               ap_uint<512>* cache_nodes_3,
                               ap_uint<512>* cache_nodes_4,
                               ap_uint<512>* cache_nodes_5,
                               ap_uint<512>* cache_nodes_6,
                               ap_uint<512>* cache_nodes_7,
                               ap_uint<512>* cache_nodes_8,
                               ap_uint<512>* cache_nodes_9,
                               ap_uint<512>* cache_nodes_10,
                               ap_uint<512>* cache_nodes_11,
                               ap_uint<512>* cache_nodes_12,
                               ap_uint<512>* cache_nodes_13,
                               ap_uint<512>* cache_nodes_14,
                               ap_uint<512>* cache_nodes_15,
                               ap_uint<512>* dag_nodes_0,
                               ap_uint<512>* dag_nodes_1,
                               ap_uint<512>* dag_nodes_2,
                               ap_uint<512>* dag_nodes_3,
                               ap_uint<512>* dag_nodes_4,
                               ap_uint<512>* dag_nodes_5,
                               ap_uint<512>* dag_nodes_6,
                               ap_uint<512>* dag_nodes_7,
                               ap_uint<512>* dag_nodes_8,
                               ap_uint<512>* dag_nodes_9,
                               ap_uint<512>* dag_nodes_10,
                               ap_uint<512>* dag_nodes_11,
                               ap_uint<512>* dag_nodes_12,
                               ap_uint<512>* dag_nodes_13,
                               ap_uint<512>* dag_nodes_14,
                               ap_uint<512>* dag_nodes_15,
                               ap_uint<32>* cfgs) {
// clang-format off

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_0 port = cache_nodes_0

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_0 port = dag_nodes_0

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_1 port = cache_nodes_1

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_1 port = dag_nodes_1


#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_2 port = cache_nodes_2

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_2 port = dag_nodes_2


#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_3 port = cache_nodes_3

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_3 port = dag_nodes_3


#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_4 port = cache_nodes_4

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_4 port = dag_nodes_4


#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_5 port = cache_nodes_5

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_5 port = dag_nodes_5


#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_6 port = cache_nodes_6

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_6 port = dag_nodes_6


#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_7 port = cache_nodes_7

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_7 port = dag_nodes_7


#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_8 port = cache_nodes_8

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_8 port = dag_nodes_8


#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_9 port = cache_nodes_9

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_9 port = dag_nodes_9

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_10 port = cache_nodes_10

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_10 port = dag_nodes_10

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_11 port = cache_nodes_11

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_11 port = dag_nodes_11


#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_12 port = cache_nodes_12

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_12 port = dag_nodes_12


#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_13 port = cache_nodes_13

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_13 port = dag_nodes_13


#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_14 port = cache_nodes_14

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_14 port = dag_nodes_14


#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_15 port = cache_nodes_15

#pragma HLS INTERFACE m_axi offset = slave latency = 128 \
    num_write_outstanding = 2 num_read_outstanding = 256 \
    max_write_burst_length = 2 max_read_burst_length = 2 \
    bundle = gmem_15 port = dag_nodes_15

#pragma HLS INTERFACE m_axi offset = slave latency = 2 \
    num_write_outstanding = 2 num_read_outstanding = 2 \
    max_write_burst_length = 3 max_read_burst_length = 2 \
    bundle = plmem_0 port = cfgs

#pragma HLS INTERFACE s_axilite port = cache_nodes_0 bundle = control
#pragma HLS INTERFACE s_axilite port = cache_nodes_1 bundle = control
#pragma HLS INTERFACE s_axilite port = cache_nodes_2 bundle = control
#pragma HLS INTERFACE s_axilite port = cache_nodes_3 bundle = control
#pragma HLS INTERFACE s_axilite port = cache_nodes_4 bundle = control
#pragma HLS INTERFACE s_axilite port = cache_nodes_5 bundle = control
#pragma HLS INTERFACE s_axilite port = cache_nodes_6 bundle = control
#pragma HLS INTERFACE s_axilite port = cache_nodes_7 bundle = control
#pragma HLS INTERFACE s_axilite port = cache_nodes_8 bundle = control
#pragma HLS INTERFACE s_axilite port = cache_nodes_9 bundle = control
#pragma HLS INTERFACE s_axilite port = cache_nodes_10 bundle = control
#pragma HLS INTERFACE s_axilite port = cache_nodes_11 bundle = control
#pragma HLS INTERFACE s_axilite port = cache_nodes_12 bundle = control
#pragma HLS INTERFACE s_axilite port = cache_nodes_13 bundle = control
#pragma HLS INTERFACE s_axilite port = cache_nodes_14 bundle = control
#pragma HLS INTERFACE s_axilite port = cache_nodes_15 bundle = control
#pragma HLS INTERFACE s_axilite port = dag_nodes_0 bundle = control
#pragma HLS INTERFACE s_axilite port = dag_nodes_1 bundle = control
#pragma HLS INTERFACE s_axilite port = dag_nodes_2 bundle = control
#pragma HLS INTERFACE s_axilite port = dag_nodes_3 bundle = control
#pragma HLS INTERFACE s_axilite port = dag_nodes_4 bundle = control
#pragma HLS INTERFACE s_axilite port = dag_nodes_5 bundle = control
#pragma HLS INTERFACE s_axilite port = dag_nodes_6 bundle = control
#pragma HLS INTERFACE s_axilite port = dag_nodes_7 bundle = control
#pragma HLS INTERFACE s_axilite port = dag_nodes_8 bundle = control
#pragma HLS INTERFACE s_axilite port = dag_nodes_9 bundle = control
#pragma HLS INTERFACE s_axilite port = dag_nodes_10 bundle = control
#pragma HLS INTERFACE s_axilite port = dag_nodes_11 bundle = control
#pragma HLS INTERFACE s_axilite port = dag_nodes_12 bundle = control
#pragma HLS INTERFACE s_axilite port = dag_nodes_13 bundle = control
#pragma HLS INTERFACE s_axilite port = dag_nodes_14 bundle = control
#pragma HLS INTERFACE s_axilite port = dag_nodes_15 bundle = control
#pragma HLS INTERFACE s_axilite port = cfgs bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control
    // clang-format on
    ap_uint<32> cache_nodes_num = cfgs[0];
    ap_uint<32> dag_start_index = cfgs[1];
    ap_uint<32> dag_node_count = cfgs[2];

    ap_uint<32> dag_bias[16];
#pragma HLS array_partition variable = dag_bias dim = 1 complete
    for (int i = 0; i < 16; i++) {
#pragma HLS pipeline II = 1
        dag_bias[i] = dag_start_index + i * dag_node_count;
    }

    wrapper(cache_nodes_0, cache_nodes_1, cache_nodes_2, cache_nodes_3, cache_nodes_4, cache_nodes_5, cache_nodes_6,
            cache_nodes_7, cache_nodes_8, cache_nodes_9, cache_nodes_10, cache_nodes_11, cache_nodes_12, cache_nodes_13,
            cache_nodes_14, cache_nodes_15, dag_nodes_0, dag_nodes_1, dag_nodes_2, dag_nodes_3, dag_nodes_4,
            dag_nodes_5, dag_nodes_6, dag_nodes_7, dag_nodes_8, dag_nodes_9, dag_nodes_10, dag_nodes_11, dag_nodes_12,
            dag_nodes_13, dag_nodes_14, dag_nodes_15, cache_nodes_num, dag_bias, dag_node_count);
}
