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

#ifndef __XILINX_DAG_KERNEL_HPP__
#define __XILINX_DAG_KERNEL_HPP__

#include <hls_stream.h>
#include <ap_int.h>
#include "sha3_ethash.hpp"
#include "dag_gen.hpp"

template <int batch_width>
void dag_gen(ap_uint<512>* cache_nodes,
             ap_uint<32> cache_nodes_num,
             ap_uint<512>* dag_nodes,
             ap_uint<32> dag_start_index,
             ap_uint<32> dag_node_count) {
    //
    const int batch = 1 << batch_width;
    //
    ap_uint<512> ret[batch];
#pragma HLS resource variable = ret core = RAM_2P_URAM
    //
    for (ap_uint<32> finish_nodes = 0; finish_nodes < dag_node_count; finish_nodes += batch) {
        // 1. before loop-256
        for (ap_uint<batch_width + 1> i = 0; i < batch; i++) {
            ap_uint<32> index = dag_start_index + finish_nodes + i;
            ap_uint<32> addr = index % cache_nodes_num;
            ap_uint<512> tmp = cache_nodes[addr] ^ index;
            ret[i] = sha3_512_64(tmp);
        }
        // 2. loop-256
        for (ap_uint<9> i = 0; i < 256; i++) {
            for (ap_uint<batch_width + 1> j = 0; j < batch; j++) {
#pragma HLS pipeline II = 1
                ap_uint<32> index = dag_start_index + finish_nodes + j;
                ap_uint<32> fnv_x = index ^ i;

                ap_uint<4> i_mod = i.range(3, 0);
                ap_uint<512> tmp = ret[j];
                ap_uint<32> fnv_y = tmp.range(32 * i_mod + 31, 32 * i_mod);

                ap_uint<32> parent_index = fnv_hash(fnv_x, fnv_y) % cache_nodes_num;

                ap_uint<512> parent_node = cache_nodes[parent_index];

                for (int k = 0; k < 16; k++) {
#pragma HLS unroll
                    tmp.range(k * 32 + 31, k * 32) =
                        fnv_hash(tmp.range(k * 32 + 31, k * 32), parent_node.range(k * 32 + 31, k * 32));
                }
                ret[j] = tmp;
            }
        }
        // 3. post loop-256 and write out
        for (ap_uint<batch_width + 1> i = 0; i < batch; i++) {
            if ((finish_nodes + i) < dag_node_count) {
                ret[i] = sha3_512_64(ret[i]);
                dag_nodes[finish_nodes + i] = ret[i];
            }
        }
    }
}

#endif
