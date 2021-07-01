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
