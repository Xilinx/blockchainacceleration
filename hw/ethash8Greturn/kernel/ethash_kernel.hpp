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


#ifndef __XILINX_ETHASHTOP_HPP__
#define __XILINX_ETHASHTOP_HPP__

#include <ap_int.h>
#include <hls_stream.h>

extern "C" void ethash_kernel0(unsigned full_size,
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

                               ap_uint<32>* ret);

extern "C" void ethash_kernel1(unsigned full_size,
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

                               ap_uint<32>* ret);

extern "C" void ethash_kernel2(unsigned full_size,
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

                               ap_uint<32>* ret);

extern "C" void ethash_kernel3(unsigned full_size,
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

                               ap_uint<32>* ret);

extern "C" void ethash_kernel4(unsigned full_size,
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

                               ap_uint<32>* ret);

extern "C" void ethash_kernel5(unsigned full_size,
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

                               ap_uint<32>* ret);

extern "C" void ethash_kernel6(unsigned full_size,
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

                               ap_uint<32>* ret);

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

                               ap_uint<32>* ret);

#endif
