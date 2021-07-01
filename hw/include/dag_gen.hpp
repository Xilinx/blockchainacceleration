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

#ifndef __XILINX_DAG_GEN_HPP__
#define __XILINX_DAG_GEN_HPP__

#include <hls_stream.h>
#include <ap_int.h>
#include "sha3_ethash.hpp"

static ap_uint<32> fnv_hash(ap_uint<32> x, ap_uint<32> y) {
    // result = (x * FNV_PRIME) ^ y;
    // the second operator is XOR, not POWR...
    const ap_uint<25> FNV_PRIME = 0x01000193;
    ap_uint<32> tmp = x * FNV_PRIME;
    return tmp ^ y;
}

void compute_cache_nodes(ap_uint<512>* cache_nodes, ap_uint<64> cache_size, ap_uint<256> seed) {
    //
    ap_uint<32> num_nodes = cache_size / 64;

    cache_nodes[0] = sha3_512_32(seed);

    for (ap_uint<32> i = 1; i < num_nodes; i++) {
        cache_nodes[i] = sha3_512_64(cache_nodes[i - 1]);
    }

    for (int round = 0; round < 3; round++) {
        for (ap_uint<32> i = 0; i < num_nodes; i++) {
            ap_uint<32> idx = cache_nodes[i].range(31, 0) % num_nodes;
            ap_uint<512> node_1 = cache_nodes[(num_nodes - 1 + i) % num_nodes];
            ap_uint<512> node_2 = cache_nodes[idx];
            node_1 ^= node_2;
            cache_nodes[i] = sha3_512_64(node_1);
        }
    }
}

ap_uint<512> compute_dag_item(ap_uint<32> node_index, ap_uint<512>* cache_nodes, ap_uint<32> cache_nodes_num) {
    //
    ap_uint<512> ret = cache_nodes[node_index % cache_nodes_num];

    ret.range(31, 0) = ret.range(31, 0) ^ node_index;

    ret = sha3_512_64(ret);

    for (ap_uint<9> i = 0; i < 256; i++) {
        ap_uint<32> fnv_x = node_index ^ i;

        ap_uint<4> i_mod = i.range(3, 0);
        ap_uint<32> fnv_y = ret.range(32 * i_mod + 31, 32 * i_mod);

        ap_uint<32> parent_index = fnv_hash(fnv_x, fnv_y) % cache_nodes_num;

        ap_uint<512> parent_node = cache_nodes[parent_index];

        for (int j = 0; j < 16; j++) {
#pragma HLS unroll
            ret.range(j * 32 + 31, j * 32) =
                fnv_hash(ret.range(j * 32 + 31, j * 32), parent_node.range(j * 32 + 31, j * 32));
        }
    }
    ret = sha3_512_64(ret);
    return ret;
}

void compute_full_dag(ap_uint<512>* dag_nodes,
                      ap_uint<64> full_size,
                      ap_uint<512>* cache_nodes,
                      ap_uint<32> cache_nodes_num) {
    //
    ap_uint<32> dag_nodes_num = full_size / 64;
    for (ap_uint<32> n = 0; n < dag_nodes_num; n++) {
        dag_nodes[n] = compute_dag_item(n, cache_nodes, cache_nodes_num);
    }
}

#endif
