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

#ifndef __XILINX_ETHASH_HPP__
#define __XILINX_ETHASH_HPP__

#ifndef __SYNTHESIS__
#include <iostream>
#endif
#include <hls_stream.h>
#include <ap_int.h>
#include "sha3_ethash.hpp"

const int NODE_WORDS = 64 / 4; // 16
const int ETHASH_MIX_BYTES = 128;
const int MIX_WORDS = ETHASH_MIX_BYTES / 4;   // 32
const int MIX_NODES = MIX_WORDS / NODE_WORDS; // 2
const int CHANNEL_NODES = (1 << 28) / 64;

static ap_uint<32> fnv_hash(ap_uint<32> x, ap_uint<32> y) {
    // result = (x * FNV_PRIME) ^ y;
    // the second operator is XOR, not POWR...
    const ap_uint<25> FNV_PRIME = 0x01000193;
    ap_uint<32> tmp = x * FNV_PRIME;
    return tmp ^ y;
}

/*
 *  * return a % b, but b[24] must be 1.
 *   */
template <int W>
inline ap_uint<W> special_mod(ap_uint<32> a, ap_uint<W> b) {
    ap_uint<33> tmp = a;
    for (int i = 0; i < (33 - W); i++) {
        if (tmp.range(32, 32 - W) >= b) {
            tmp.range(32, 32 - W) = tmp.range(32, 32 - W) - b;
        }
        tmp <<= 1;
    }
    return tmp.range(32, 33 - W);
}

inline void nodeLookup(ap_uint<512>* full_node,
                       hls::stream<ap_uint<25> >& indexStrm,
                       hls::stream<ap_uint<256> >& dagStrm0,
                       hls::stream<ap_uint<256> >& dagStrm1,
                       hls::stream<ap_uint<256> >& dagStrm2,
                       hls::stream<ap_uint<256> >& dagStrm3) {
    ap_uint<25> index = 0;

    while (index[24] != 1) {
//#pragma HLS PIPELINE II = 1
//        index = indexStrm.read();
//        ap_uint<1024> tmp = full_node[index.range(20, 0)];
//        dagStrm0.write(tmp.range(255, 0));
//        dagStrm1.write(tmp.range(511, 256));
//        dagStrm2.write(tmp.range(767, 512));
//        dagStrm3.write(tmp.range(1023, 768));
#pragma HLS PIPELINE II = 2
        index = indexStrm.read();
        ap_uint<25> index_for_512 = index << 1;
        ap_uint<512> dag_item[2];
        for (int i = 0; i < 2; i++) {
            dag_item[i] = full_node[index_for_512 + i];
        }
        ap_uint<1024> combined_dag_item;
        combined_dag_item.range(511, 0) = dag_item[0];
        combined_dag_item.range(1023, 512) = dag_item[1];
        dagStrm0.write(combined_dag_item.range(255, 0));
        dagStrm1.write(combined_dag_item.range(511, 256));
        dagStrm2.write(combined_dag_item.range(767, 512));
        dagStrm3.write(combined_dag_item.range(1023, 768));
    }
}

inline void prefnv(ap_uint<32> full_size,
                   ap_uint<6> iter,

                   hls::stream<ap_uint<256> >& inStrm0,
                   hls::stream<ap_uint<256> >& inStrm1,

                   ap_uint<32>* s0ping,
                   ap_uint<512>* smix0ping,
                   ap_uint<512>* smix1ping,

                   hls::stream<ap_uint<2> >& orderStrm,
                   hls::stream<ap_uint<25> >& idxOutStrm0,
                   hls::stream<ap_uint<25> >& idxOutStrm1,
                   hls::stream<ap_uint<25> >& idxOutStrm2,
                   hls::stream<ap_uint<25> >& idxOutStrm3,

                   hls::stream<ap_uint<32> >& s0OutStrm,
                   hls::stream<ap_uint<256> >& smix0OutStrm0,
                   hls::stream<ap_uint<256> >& smix0OutStrm1,
                   hls::stream<ap_uint<256> >& smix1OutStrm0,
                   hls::stream<ap_uint<256> >& smix1OutStrm1) {
    //    const unsigned int page_size = 128;
    const unsigned int num_full_pages = full_size; // page_size;

    ap_uint<32> index;
    ap_uint<32> s0;
    ap_uint<512> smix0;
    ap_uint<512> smix1;
    ap_uint<2> addrH;
    ap_uint<25> addrL;

    for (ap_uint<32> i = 0; i < 4096; i++) {
#pragma HLS PIPELINE II = 1
        if (iter == 0) {
            ap_uint<256> reg0 = inStrm0.read();
            ap_uint<256> reg1 = inStrm1.read();
            s0 = reg0.range(31, 0);
            smix0.range(255, 0) = reg0;
            smix0.range(511, 256) = reg1;
            smix1.range(255, 0) = reg0;
            smix1.range(511, 256) = reg1;
        } else {
            s0 = s0ping[i];
            smix0 = smix0ping[i];
            smix1 = smix1ping[i];
        }

        ap_uint<32> fnv_x = s0.range(31, 0) ^ iter;
        ap_uint<32> fnv_t1 = iter[4];
        ap_uint<32> fnv_t2 = iter.range(3, 0);

        ap_uint<512> pre_fnv_y = fnv_t1 == 0 ? smix0 : smix1;

        ap_uint<32> pre_fnv_y_array[16];
#pragma HLS array_partition variable = pre_fnv_y_array dim = 1 complete
        for (int j = 0; j < 16; j++) {
#pragma HLS unroll
            pre_fnv_y_array[j] = pre_fnv_y.range(j * 32 + 31, j * 32);
        }

        ap_uint<32> fnv_y = pre_fnv_y_array[fnv_t2];
        ap_uint<32> index = special_mod<26>(fnv_hash(fnv_x, fnv_y), num_full_pages);
        // Template 26 for 8G dagsize. if 4G should be 25.

        ap_uint<12> index_remapL = index.range(11, 0);
        ap_uint<2> index_remapM = index.range(13, 12);
        ap_uint<12> index_remapH = index.range(25, 14);
        ap_uint<12> index_remapHR = index_remapH.reverse();

        addrH = index_remapM;
        addrL.range(23, 0) = index_remapHR.concat(index_remapL);
        addrL[24] = 0;
        // interleave the memory to fully use HBM bandwidth.

        if (addrH == 0) {
            idxOutStrm0.write(addrL);
        } else if (addrH == 1) {
            idxOutStrm1.write(addrL);
        } else if (addrH == 2) {
            idxOutStrm2.write(addrL);
        } else {
            idxOutStrm3.write(addrL);
        }
        orderStrm.write(addrH);
        // send the order so the post fnv can recover the order.

        s0OutStrm.write(s0);
        smix0OutStrm0.write(smix0.range(255, 0));
        smix0OutStrm1.write(smix0.range(511, 256));
        smix1OutStrm0.write(smix1.range(255, 0));
        smix1OutStrm1.write(smix1.range(511, 256));
    }

    addrL[24] = 1;
    idxOutStrm0.write(addrL);
    idxOutStrm1.write(addrL);
    idxOutStrm2.write(addrL);
    idxOutStrm3.write(addrL);
}

inline void postfnv(ap_uint<6> iter,

                    hls::stream<ap_uint<32> >& s0InStrm,
                    hls::stream<ap_uint<256> >& smix0InStrm0,
                    hls::stream<ap_uint<256> >& smix0InStrm1,
                    hls::stream<ap_uint<256> >& smix1InStrm0,
                    hls::stream<ap_uint<256> >& smix1InStrm1,

                    hls::stream<ap_uint<2> >& orderStrm,
                    hls::stream<ap_uint<256> >& dagInStrm00,
                    hls::stream<ap_uint<256> >& dagInStrm01,
                    hls::stream<ap_uint<256> >& dagInStrm02,
                    hls::stream<ap_uint<256> >& dagInStrm03,

                    hls::stream<ap_uint<256> >& dagInStrm10,
                    hls::stream<ap_uint<256> >& dagInStrm11,
                    hls::stream<ap_uint<256> >& dagInStrm12,
                    hls::stream<ap_uint<256> >& dagInStrm13,

                    hls::stream<ap_uint<256> >& dagInStrm20,
                    hls::stream<ap_uint<256> >& dagInStrm21,
                    hls::stream<ap_uint<256> >& dagInStrm22,
                    hls::stream<ap_uint<256> >& dagInStrm23,

                    hls::stream<ap_uint<256> >& dagInStrm30,
                    hls::stream<ap_uint<256> >& dagInStrm31,
                    hls::stream<ap_uint<256> >& dagInStrm32,
                    hls::stream<ap_uint<256> >& dagInStrm33,

                    ap_uint<32>* s0pong,
                    ap_uint<512>* smix0pong,
                    ap_uint<512>* smix1pong,

                    hls::stream<ap_uint<256> >& outStrm00,
                    hls::stream<ap_uint<256> >& outStrm01,
                    hls::stream<ap_uint<256> >& outStrm10,
                    hls::stream<ap_uint<256> >& outStrm11) {
    for (ap_uint<32> i = 0; i < 4096; i++) {
#pragma HLS PIPELINE II = 1
        ap_uint<2> order = orderStrm.read();
        ap_uint<1024> dag_r;
        if (order == 0) {
            dag_r.range(255, 0) = dagInStrm00.read();
            dag_r.range(511, 256) = dagInStrm01.read();
            dag_r.range(767, 512) = dagInStrm02.read();
            dag_r.range(1023, 768) = dagInStrm03.read();
        } else if (order == 1) {
            dag_r.range(255, 0) = dagInStrm10.read();
            dag_r.range(511, 256) = dagInStrm11.read();
            dag_r.range(767, 512) = dagInStrm12.read();
            dag_r.range(1023, 768) = dagInStrm13.read();
        } else if (order == 2) {
            dag_r.range(255, 0) = dagInStrm20.read();
            dag_r.range(511, 256) = dagInStrm21.read();
            dag_r.range(767, 512) = dagInStrm22.read();
            dag_r.range(1023, 768) = dagInStrm23.read();
        } else if (order == 3) {
            dag_r.range(255, 0) = dagInStrm30.read();
            dag_r.range(511, 256) = dagInStrm31.read();
            dag_r.range(767, 512) = dagInStrm32.read();
            dag_r.range(1023, 768) = dagInStrm33.read();
        }
        // recover the order

        ap_uint<512> smix0;
        smix0.range(255, 0) = smix0InStrm0.read();
        smix0.range(511, 256) = smix0InStrm1.read();
        ap_uint<512> smix1;
        smix1.range(255, 0) = smix1InStrm0.read();
        smix1.range(511, 256) = smix1InStrm1.read();

        ap_uint<512> dag0 = dag_r.range(511, 0);
        ap_uint<512> dag1 = dag_r.range(1023, 512);

        ap_uint<512> smix0nxt;
        ap_uint<512> smix1nxt;

        for (int k = 0; k < NODE_WORDS; k++) {
#pragma HLS unroll
            smix0nxt.range(k * 32 + 31, k * 32) =
                fnv_hash(smix0.range(k * 32 + 31, k * 32), dag0.range(k * 32 + 31, k * 32));
        }

        for (int k = 0; k < NODE_WORDS; k++) {
#pragma HLS unroll
            smix1nxt.range(k * 32 + 31, k * 32) =
                fnv_hash(smix1.range(k * 32 + 31, k * 32), dag1.range(k * 32 + 31, k * 32));
        }

        if (iter == 63) { // final iter output the result outside
            s0InStrm.read();
            outStrm00.write(smix0nxt.range(255, 0));
            outStrm01.write(smix0nxt.range(511, 256));
            outStrm10.write(smix1nxt.range(255, 0));
            outStrm11.write(smix1nxt.range(511, 256));
        } else {
            s0pong[i] = s0InStrm.read();
            smix0pong[i] = smix0nxt;
            smix1pong[i] = smix1nxt;
        }
    }

    dagInStrm00.read();
    dagInStrm01.read();
    dagInStrm02.read();
    dagInStrm03.read();
    dagInStrm10.read();
    dagInStrm11.read();
    dagInStrm12.read();
    dagInStrm13.read();
    dagInStrm20.read();
    dagInStrm21.read();
    dagInStrm22.read();
    dagInStrm23.read();
    dagInStrm30.read();
    dagInStrm31.read();
    dagInStrm32.read();
    dagInStrm33.read();
}

inline void dagAccess(ap_uint<32> full_size,
                      ap_uint<6> iter,

                      ap_uint<512>* dram0,
                      ap_uint<512>* dram1,
                      ap_uint<512>* dram2,
                      ap_uint<512>* dram3,

                      hls::stream<ap_uint<256> >& inStrm0,
                      hls::stream<ap_uint<256> >& inStrm1,

                      ap_uint<32>* s0ping,
                      ap_uint<512>* smix0ping,
                      ap_uint<512>* smix1ping,

                      ap_uint<32>* s0pong,
                      ap_uint<512>* smix0pong,
                      ap_uint<512>* smix1pong,

                      hls::stream<ap_uint<256> >& outStrm00,
                      hls::stream<ap_uint<256> >& outStrm01,
                      hls::stream<ap_uint<256> >& outStrm10,
                      hls::stream<ap_uint<256> >& outStrm11) {
#pragma HLS DATAFLOW

    hls::stream<ap_uint<2> > orderpreStrm;
#pragma HLS stream variable = orderpreStrm depth = 1024
#pragma HLS resource variable = orderpreStrm core = FIFO_BRAM

    hls::stream<ap_uint<25> > indexpreStrm0;
#pragma HLS stream variable = indexpreStrm0 depth = 1024
#pragma HLS resource variable = indexpreStrm0 core = FIFO_BRAM

    hls::stream<ap_uint<25> > indexpreStrm1;
#pragma HLS stream variable = indexpreStrm1 depth = 1024
#pragma HLS resource variable = indexpreStrm1 core = FIFO_BRAM

    hls::stream<ap_uint<25> > indexpreStrm2;
#pragma HLS stream variable = indexpreStrm2 depth = 1024
#pragma HLS resource variable = indexpreStrm2 core = FIFO_BRAM

    hls::stream<ap_uint<25> > indexpreStrm3;
#pragma HLS stream variable = indexpreStrm3 depth = 1024
#pragma HLS resource variable = indexpreStrm3 core = FIFO_BRAM

    hls::stream<ap_uint<32> > s0preStrm0;
#pragma HLS stream variable = s0preStrm0 depth = 1024
#pragma HLS resource variable = s0preStrm0 core = FIFO_BRAM

    hls::stream<ap_uint<256> > smix0preStrm0; // use two 256 bit stream to transfer smix0 for better timing
#pragma HLS stream variable = smix0preStrm0 depth = 1024
// This stream bypass the nodelookup function. So it is deeper to avoid deadlock
#pragma HLS resource variable = smix0preStrm0 core = FIFO_BRAM

    hls::stream<ap_uint<256> > smix0preStrm1;
#pragma HLS stream variable = smix0preStrm1 depth = 1024
#pragma HLS resource variable = smix0preStrm1 core = FIFO_BRAM

    hls::stream<ap_uint<256> > smix1preStrm0;
#pragma HLS stream variable = smix1preStrm0 depth = 1024
#pragma HLS resource variable = smix1preStrm0 core = FIFO_BRAM

    hls::stream<ap_uint<256> > smix1preStrm1;
#pragma HLS stream variable = smix1preStrm1 depth = 1024
#pragma HLS resource variable = smix1preStrm1 core = FIFO_BRAM

    prefnv(full_size, iter, inStrm0, inStrm1, s0ping, smix0ping, smix1ping, orderpreStrm, indexpreStrm0, indexpreStrm1,
           indexpreStrm2, indexpreStrm3, s0preStrm0, smix0preStrm0, smix0preStrm1, smix1preStrm0, smix1preStrm1);

    hls::stream<ap_uint<256> > dagStrm00;
#pragma HLS stream variable = dagStrm00 depth = 16
// depth shall be deeper for better efficiency but drop freq. 32 is better if do not take timing into consideration.
// depth beyond 32 nouse due to batch process.
#pragma HLS resource variable = dagStrm00 core = FIFO_SRL

    hls::stream<ap_uint<256> > dagStrm01;
#pragma HLS stream variable = dagStrm01 depth = 16
#pragma HLS resource variable = dagStrm01 core = FIFO_SRL

    hls::stream<ap_uint<256> > dagStrm02;
#pragma HLS stream variable = dagStrm02 depth = 16
#pragma HLS resource variable = dagStrm02 core = FIFO_SRL

    hls::stream<ap_uint<256> > dagStrm03;
#pragma HLS stream variable = dagStrm03 depth = 16
#pragma HLS resource variable = dagStrm03 core = FIFO_SRL

    hls::stream<ap_uint<256> > dagStrm10;
#pragma HLS stream variable = dagStrm10 depth = 16
#pragma HLS resource variable = dagStrm10 core = FIFO_SRL

    hls::stream<ap_uint<256> > dagStrm11;
#pragma HLS stream variable = dagStrm11 depth = 16
#pragma HLS resource variable = dagStrm11 core = FIFO_SRL

    hls::stream<ap_uint<256> > dagStrm12;
#pragma HLS stream variable = dagStrm12 depth = 16
#pragma HLS resource variable = dagStrm12 core = FIFO_SRL

    hls::stream<ap_uint<256> > dagStrm13;
#pragma HLS stream variable = dagStrm13 depth = 16
#pragma HLS resource variable = dagStrm13 core = FIFO_SRL

    hls::stream<ap_uint<256> > dagStrm20;
#pragma HLS stream variable = dagStrm20 depth = 16
#pragma HLS resource variable = dagStrm20 core = FIFO_SRL

    hls::stream<ap_uint<256> > dagStrm21;
#pragma HLS stream variable = dagStrm21 depth = 16
#pragma HLS resource variable = dagStrm21 core = FIFO_SRL

    hls::stream<ap_uint<256> > dagStrm22;
#pragma HLS stream variable = dagStrm22 depth = 16
#pragma HLS resource variable = dagStrm22 core = FIFO_SRL

    hls::stream<ap_uint<256> > dagStrm23;
#pragma HLS stream variable = dagStrm23 depth = 16
#pragma HLS resource variable = dagStrm23 core = FIFO_SRL

    hls::stream<ap_uint<256> > dagStrm30;
#pragma HLS stream variable = dagStrm30 depth = 16
#pragma HLS resource variable = dagStrm30 core = FIFO_SRL

    hls::stream<ap_uint<256> > dagStrm31;
#pragma HLS stream variable = dagStrm31 depth = 16
#pragma HLS resource variable = dagStrm31 core = FIFO_SRL

    hls::stream<ap_uint<256> > dagStrm32;
#pragma HLS stream variable = dagStrm32 depth = 16
#pragma HLS resource variable = dagStrm32 core = FIFO_SRL

    hls::stream<ap_uint<256> > dagStrm33;
#pragma HLS stream variable = dagStrm33 depth = 16
#pragma HLS resource variable = dagStrm33 core = FIFO_SRL

    nodeLookup(dram0, indexpreStrm0, dagStrm00, dagStrm01, dagStrm02, dagStrm03);
    nodeLookup(dram1, indexpreStrm1, dagStrm10, dagStrm11, dagStrm12, dagStrm13);
    nodeLookup(dram2, indexpreStrm2, dagStrm20, dagStrm21, dagStrm22, dagStrm23);
    nodeLookup(dram3, indexpreStrm3, dagStrm30, dagStrm31, dagStrm32, dagStrm33);

    postfnv(iter, s0preStrm0, smix0preStrm0, smix0preStrm1, smix1preStrm0, smix1preStrm1, orderpreStrm, dagStrm00,
            dagStrm01, dagStrm02, dagStrm03, dagStrm10, dagStrm11, dagStrm12, dagStrm13, dagStrm20, dagStrm21,
            dagStrm22, dagStrm23, dagStrm30, dagStrm31, dagStrm32, dagStrm33, s0pong, smix0pong, smix1pong, outStrm00,
            outStrm01, outStrm10, outStrm11);
}

inline void mixdag(ap_uint<32> batchCnt,
                   ap_uint<32> full_size,

                   ap_uint<512>* dram0,
                   ap_uint<512>* dram1,
                   ap_uint<512>* dram2,
                   ap_uint<512>* dram3,

                   hls::stream<ap_uint<256> >& inStrm0,
                   hls::stream<ap_uint<256> >& inStrm1,

                   hls::stream<ap_uint<256> >& outStrm00,
                   hls::stream<ap_uint<256> >& outStrm01,
                   hls::stream<ap_uint<256> >& outStrm10,
                   hls::stream<ap_uint<256> >& outStrm11) {
#ifndef __SYNTHESIS__
    ap_uint<512>* smix0ping = (ap_uint<512>*)malloc(4096 * sizeof(ap_uint<512>));
    ap_uint<512>* smix0pong = (ap_uint<512>*)malloc(4096 * sizeof(ap_uint<512>));

    ap_uint<512>* smix1ping = (ap_uint<512>*)malloc(4096 * sizeof(ap_uint<512>));
    ap_uint<512>* smix1pong = (ap_uint<512>*)malloc(4096 * sizeof(ap_uint<512>));

    ap_uint<32>* s0ping = (ap_uint<32>*)malloc(4096 * sizeof(ap_uint<32>));
    ap_uint<32>* s0pong = (ap_uint<32>*)malloc(4096 * sizeof(ap_uint<32>));
#else
    ap_uint<512> smix0ping[4096];
#pragma HLS resource variable = smix0ping core = RAM_2P_URAM

    ap_uint<512> smix0pong[4096];
#pragma HLS resource variable = smix0pong core = RAM_2P_URAM

    ap_uint<512> smix1ping[4096];
#pragma HLS resource variable = smix1ping core = RAM_2P_URAM

    ap_uint<512> smix1pong[4096];
#pragma HLS resource variable = smix1pong core = RAM_2P_URAM

    ap_uint<32> s0ping[4096];
#pragma HLS resource variable = s0ping core = RAM_2P_URAM

    ap_uint<32> s0pong[4096];
#pragma HLS resource variable = s0pong core = RAM_2P_URAM

#endif

    for (int k = 0; k < batchCnt; k++) {
        for (ap_uint<8> i = 0; i < 64; i++) { // ping pong the URAM
            if (i[0] == 0) {
                dagAccess(full_size, i, dram0, dram1, dram2, dram3,

                          inStrm0, inStrm1,

                          s0ping, smix0ping, smix1ping,

                          s0pong, smix0pong, smix1pong,

                          outStrm00, outStrm01, outStrm10, outStrm11);
            } else {
                dagAccess(full_size, i, dram0, dram1, dram2, dram3,

                          inStrm0, inStrm1,

                          s0pong, smix0pong, smix1pong,

                          s0ping, smix0ping, smix1ping,

                          outStrm00, outStrm01, outStrm10, outStrm11);
            }
        }
    }
}

inline void sha3_512_40_strm(ap_uint<256> header_hash,
                             ap_uint<64> nonce_start,
                             ap_uint<32> batch_cnt,

                             hls::stream<ap_uint<256> >& outStrm0,
                             hls::stream<ap_uint<256> >& outStrm1) {
    for (ap_uint<32> i = 0; i < batch_cnt * 4096; i++) {
#pragma HLS PIPELINE off
        ap_uint<512> s0;
        s0.range(255, 0) = header_hash;
        s0.range(319, 256) = nonce_start + i;
        ap_uint<512> tmp = sha3_512_40(s0);
        outStrm0.write(tmp.range(255, 0));
        outStrm1.write(tmp.range(511, 256));
    }
}

inline void compress_strm(ap_uint<32> batch_cnt,
                          hls::stream<ap_uint<256> >& inStrm00,
                          hls::stream<ap_uint<256> >& inStrm01,
                          hls::stream<ap_uint<256> >& inStrm10,
                          hls::stream<ap_uint<256> >& inStrm11,

                          hls::stream<ap_uint<256> >& outStrm) {
    ap_uint<512> s_mix[2] = {0, 0};
    ap_uint<32> s0;
    ap_uint<256> res_hash;

    for (int i = 0; i < batch_cnt * 4096; i++) {
#pragma HLS PIPELINE II = 1
        s_mix[0].range(255, 0) = inStrm00.read();
        s_mix[0].range(511, 256) = inStrm01.read();
        s_mix[1].range(255, 0) = inStrm10.read();
        s_mix[1].range(511, 256) = inStrm11.read();

        for (int w = 0; w < MIX_WORDS; w += 4) {
#pragma HLS unroll
            const ap_uint<32> t1 = w / NODE_WORDS;
            const ap_uint<32> t2 = w % NODE_WORDS;
            const ap_uint<512> local_mix = t1 == 0 ? s_mix[0] : s_mix[1];

            ap_uint<32> reduction = local_mix.range(t2 * 32 + 31, t2 * 32);
            reduction = fnv_hash(reduction, local_mix.range((t2 + 1) * 32 + 31, (t2 + 1) * 32));
            reduction = fnv_hash(reduction, local_mix.range((t2 + 2) * 32 + 31, (t2 + 2) * 32));
            reduction = fnv_hash(reduction, local_mix.range((t2 + 3) * 32 + 31, (t2 + 3) * 32));
            res_hash.range((w / 4) * 32 + 31, (w / 4) * 32) = reduction;
        }
        outStrm.write(res_hash);
    }
}

inline void sha3_512_40_strm(ap_uint<256> header_hash,
                             ap_uint<64> nonce_start,
                             ap_uint<32> batch_cnt,
                             hls::stream<ap_uint<256> >& mixInstrm,

                             hls::stream<ap_uint<256> >& s0Strm0,
                             hls::stream<ap_uint<256> >& s0Strm1,
                             hls::stream<ap_uint<256> >& mixOutStrm) {
    for (int i = 0; i < batch_cnt * 4096; i++) {
#pragma HLS PIPELINE off
        ap_uint<512> s0;
        s0.range(255, 0) = header_hash;
        s0.range(319, 256) = nonce_start + i;
        ap_uint<512> tmp = sha3_512_40(s0);

        s0Strm0.write(tmp.range(255, 0));
        s0Strm1.write(tmp.range(511, 256));
        mixOutStrm.write(mixInstrm.read());
    }
}

inline void sha3_256_96_strm(ap_uint<32> batch_cnt,
                             hls::stream<ap_uint<256> >& s0Strm0,
                             hls::stream<ap_uint<256> >& s0Strm1,
                             hls::stream<ap_uint<256> >& mixInStrm,

                             hls::stream<ap_uint<256> >& mixhashStrm,
                             hls::stream<ap_uint<256> >& resultStrm) {
    for (int i = 0; i < batch_cnt * 4096; i++) {
#pragma HLS PIPELINE off
        ap_uint<512> s0;
        ap_uint<512> mix;
        ap_uint<32> nonce;
        s0.range(255, 0) = s0Strm0.read();
        s0.range(511, 256) = s0Strm1.read();
        mix.range(255, 0) = mixInStrm.read();
        mix.range(511, 256) = 0;
        ap_uint<256> tmp = sha3_256_96(s0, mix);
        mixhashStrm.write(mix.range(255, 0));
        resultStrm.write(tmp);
    }
}

inline ap_uint<256> reorder_unit(ap_uint<256> in) {
#pragma HLS inline
    ap_uint<256> out;

    out(255, 248) = in(7, 0);
    out(247, 240) = in(15, 8);
    out(239, 232) = in(23, 16);
    out(231, 224) = in(31, 24);
    out(223, 216) = in(39, 32);
    out(215, 208) = in(47, 40);
    out(207, 200) = in(55, 48);
    out(199, 192) = in(63, 56);
    out(191, 184) = in(71, 64);
    out(183, 176) = in(79, 72);
    out(175, 168) = in(87, 80);
    out(167, 160) = in(95, 88);
    out(159, 152) = in(103, 96);
    out(151, 144) = in(111, 104);
    out(143, 136) = in(119, 112);
    out(135, 128) = in(127, 120);
    out(127, 120) = in(135, 128);
    out(119, 112) = in(143, 136);
    out(111, 104) = in(151, 144);
    out(103, 96) = in(159, 152);
    out(95, 88) = in(167, 160);
    out(87, 80) = in(175, 168);
    out(79, 72) = in(183, 176);
    out(71, 64) = in(191, 184);
    out(63, 56) = in(199, 192);
    out(55, 48) = in(207, 200);
    out(47, 40) = in(215, 208);
    out(39, 32) = in(223, 216);
    out(31, 24) = in(231, 224);
    out(23, 16) = in(239, 232);
    out(15, 8) = in(247, 240);
    out(7, 0) = in(255, 248);
    return out;
}

inline void check(ap_uint<32> batch_cnt,
                  ap_uint<256> boundary,
                  hls::stream<ap_uint<256> >& mixhashStrm,
                  hls::stream<ap_uint<256> >& resultStrm,

                  ap_uint<32>* resultOut) {
    resultOut[0][31] = 1;

    ap_uint<8> total = 0;
    for (int i = 0; i < batch_cnt * 4096; i++) {
#pragma HLS PIPELINE II = 1
        ap_uint<256> result;
        ap_uint<256> mixhash;
        result = resultStrm.read();
        mixhash = mixhashStrm.read();
        ap_uint<256> reorder = reorder_unit(result);
        if (reorder <= boundary) {
            resultOut[0] = i;
            resultOut[1] = mixhash.range(31, 0);
            resultOut[2] = mixhash.range(63, 32);
            resultOut[3] = mixhash.range(95, 64);
            resultOut[4] = mixhash.range(127, 96);
            resultOut[5] = mixhash.range(159, 128);
            resultOut[6] = mixhash.range(191, 160);
            resultOut[7] = mixhash.range(223, 192);
            resultOut[8] = mixhash.range(255, 224);
            resultOut[9] = result.range(31, 0);
            resultOut[10] = result.range(63, 32);
            resultOut[11] = result.range(95, 64);
            resultOut[12] = result.range(127, 96);
            resultOut[13] = result.range(159, 128);
            resultOut[14] = result.range(191, 160);
            resultOut[15] = result.range(223, 192);
            resultOut[16] = result.range(255, 224);
        }
    }
}

inline void ethashcore(unsigned full_size,
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

                       ap_uint<32>* resultOut) {
#pragma HLS dataflow

    ap_uint<256> header_hash_ap; // the AXI Lite can only transfer 32bit data. So combine them back to 256bit
    header_hash_ap.range(31, 0) = header_hash0;
    header_hash_ap.range(63, 32) = header_hash1;
    header_hash_ap.range(95, 64) = header_hash2;
    header_hash_ap.range(127, 96) = header_hash3;
    header_hash_ap.range(159, 128) = header_hash4;
    header_hash_ap.range(191, 160) = header_hash5;
    header_hash_ap.range(223, 192) = header_hash6;
    header_hash_ap.range(255, 224) = header_hash7;

    ap_uint<256> boundary_ap;
    boundary_ap.range(31, 0) = boundary0;
    boundary_ap.range(63, 32) = boundary1;
    boundary_ap.range(95, 64) = boundary2;
    boundary_ap.range(127, 96) = boundary3;
    boundary_ap.range(159, 128) = boundary4;
    boundary_ap.range(191, 160) = boundary5;
    boundary_ap.range(223, 192) = boundary6;
    boundary_ap.range(255, 224) = boundary7;

    ap_uint<64> nonce_start_ap;
    nonce_start_ap.range(31, 0) = nonce_start0;
    nonce_start_ap.range(63, 32) = nonce_start1;

    ap_uint<32> batch_cnt_ap = batch_cnt;
    ap_uint<32> full_size_ap = full_size;

    hls::stream<ap_uint<256> > sha3_512_Strm0;
#pragma HLS stream variable = sha3_512_Strm0 depth = 4096
#pragma HLS resource variable = sha3_512_Strm0 core = FIFO_URAM

    hls::stream<ap_uint<256> > sha3_512_Strm1;
#pragma HLS stream variable = sha3_512_Strm1 depth = 4096
#pragma HLS resource variable = sha3_512_Strm1 core = FIFO_URAM

    sha3_512_40_strm(header_hash_ap, nonce_start_ap, batch_cnt_ap, sha3_512_Strm0, sha3_512_Strm1);
#ifndef __SYNTHESIS__
    std::cout << "sha3_512 pre finish" << std::endl;
#endif

    hls::stream<ap_uint<256> > mixStrm00;
#pragma HLS stream variable = mixStrm00 depth = 8
#pragma HLS resource variable = mixStrm00 core = FIFO_SRL

    hls::stream<ap_uint<256> > mixStrm01;
#pragma HLS stream variable = mixStrm01 depth = 8
#pragma HLS resource variable = mixStrm01 core = FIFO_SRL

    hls::stream<ap_uint<256> > mixStrm10;
#pragma HLS stream variable = mixStrm10 depth = 8
#pragma HLS resource variable = mixStrm10 core = FIFO_SRL

    hls::stream<ap_uint<256> > mixStrm11;
#pragma HLS stream variable = mixStrm11 depth = 8
#pragma HLS resource variable = mixStrm11 core = FIFO_SRL
    mixdag(batch_cnt_ap, full_size_ap, dram0, dram1, dram2, dram3, sha3_512_Strm0, sha3_512_Strm1, mixStrm00, mixStrm01,
           mixStrm10, mixStrm11); // The dag access is inside this function
#ifndef __SYNTHESIS__
    std::cout << "mixdag finish" << std::endl;
#endif

    hls::stream<ap_uint<256> > compressStrm;
#pragma HLS stream variable = compressStrm depth = 4096
#pragma HLS resource variable = compressStrm core = FIFO_URAM

    compress_strm(batch_cnt_ap, mixStrm00, mixStrm01, mixStrm10, mixStrm11, compressStrm);
#ifndef __SYNTHESIS__
    std::cout << "compress finish" << std::endl;
#endif

    hls::stream<ap_uint<256> > sha512Strm0;
#pragma HLS stream variable = sha512Strm0 depth = 8
#pragma HLS resource variable = sha512Strm0 core = FIFO_SRL

    hls::stream<ap_uint<256> > sha512Strm1;
#pragma HLS stream variable = sha512Strm1 depth = 8
#pragma HLS resource variable = sha512Strm1 core = FIFO_SRL

    hls::stream<ap_uint<256> > mixStrm;
#pragma HLS stream variable = mixStrm depth = 8
#pragma HLS resource variable = mixStrm core = FIFO_SRL

    sha3_512_40_strm(header_hash_ap, nonce_start_ap, batch_cnt_ap, compressStrm, sha512Strm0, sha512Strm1, mixStrm);
#ifndef __SYNTHESIS__
    std::cout << "sha3_512 post finish" << std::endl;
#endif

    hls::stream<ap_uint<256> > mixhashStrm("mixhash");
#pragma HLS stream variable = mixhashStrm depth = 8
#pragma HLS resource variable = mixhashStrm core = FIFO_LUTRAM

    hls::stream<ap_uint<256> > resultStrm("result");
#pragma HLS stream variable = resultStrm depth = 8
#pragma HLS resource variable = resultStrm core = FIFO_LUTRAM

    sha3_256_96_strm(batch_cnt_ap, sha512Strm0, sha512Strm1, mixStrm, mixhashStrm, resultStrm);
#ifndef __SYNTHESIS__
    std::cout << "sha3_256 finish" << std::endl;
#endif

    check(batch_cnt_ap, boundary_ap, mixhashStrm, resultStrm, resultOut);
#ifndef __SYNTHESIS__
    std::cout << "check finish" << std::endl;
#endif
}

#endif
