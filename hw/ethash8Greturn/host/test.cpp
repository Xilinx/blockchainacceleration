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

#ifndef HLS_TEST
#include "xcl2.hpp"
#else
#include "ethash_kernel.hpp"
#endif

#include "sha3_ethash.hpp"

#include <algorithm>
#include <iterator>

#include <sys/time.h>
#include <new>
#include <cstdlib>
#include <ap_int.h>
#include <iostream>

#include <vector>
#include <string>
#include <fstream>

static ap_uint<32> fnv_hash_host(ap_uint<32> x, ap_uint<32> y) {
    // result = (x * FNV_PRIME) ^ y;
    // the second operator is XOR, not POWR...
    const ap_uint<25> FNV_PRIME = 0x01000193;
    ap_uint<32> tmp = x * FNV_PRIME;
    return tmp ^ y;
}

inline void ethash_naive(ap_uint<64> cnt,
                         ap_uint<256>& result,
                         ap_uint<256>& mix_hash,
                         ap_uint<512>* full_node,
                         ap_uint<64> full_size,
                         ap_uint<256> header_hash,
                         ap_uint<64> nonce) {
    // 1. pack hash and nonce together into first 40 bytes of s_mix
    ap_uint<512> s_mix[3];
    //#pragma HLS array_partition variable = s_mix dim = 1
    for (int i = 0; i < 3; i++) {
#pragma HLS unroll
        s_mix[3] = 0;
    }
    s_mix[0].range(255, 0) = header_hash;
    s_mix[0].range(319, 256) = nonce;

    // 2. compute sha3-512 hash and replicate across mix
    s_mix[0] = sha3_512_40(s_mix[0]);
    s_mix[1] = s_mix[0];
    s_mix[2] = s_mix[0];

    const unsigned int page_size = 128;
    const unsigned int num_full_pages = full_size / page_size;

    for (ap_uint<32> i = 0; i < 64; i++) {
#pragma HLS pipeline
        const ap_uint<32> fnv_x = s_mix[0].range(31, 0) ^ i;
        const ap_uint<32> fnv_t1 = (i % 32) / 16;
        const ap_uint<32> fnv_t2 = (i % 32) % 16;
        const ap_uint<32> fnv_y =
            fnv_t1 == 0 ? s_mix[1].range(fnv_t2 * 32 + 31, fnv_t2 * 32) : s_mix[2].range(fnv_t2 * 32 + 31, fnv_t2 * 32);
        const ap_uint<32> index = fnv_hash_host(fnv_x, fnv_y) % num_full_pages;

        for (int j = 0; j < 2; j++) {
#pragma HLS unroll
            ap_uint<512> dag_node = full_node[index * 2 + j];

            for (int k = 0; k < 16; k++) {
#pragma HLS unroll
                s_mix[j + 1].range(k * 32 + 31, k * 32) =
                    fnv_hash_host(s_mix[j + 1].range(k * 32 + 31, k * 32), dag_node.range(k * 32 + 31, k * 32));
            }
        }
    }

    // 3. compress mix
    ap_uint<256> res_hash;
    for (int w = 0; w < 32; w += 4) {
#pragma HLS unroll
        const ap_uint<32> t1 = w / 16;
        const ap_uint<32> t2 = w % 16;
        const ap_uint<512> local_mix = t1 == 0 ? s_mix[1] : s_mix[2];

        ap_uint<32> reduction = local_mix.range(t2 * 32 + 31, t2 * 32);
        reduction = fnv_hash_host(reduction, local_mix.range((t2 + 1) * 32 + 31, (t2 + 1) * 32));
        reduction = fnv_hash_host(reduction, local_mix.range((t2 + 2) * 32 + 31, (t2 + 2) * 32));
        reduction = fnv_hash_host(reduction, local_mix.range((t2 + 3) * 32 + 31, (t2 + 3) * 32));
        res_hash.range((w / 4) * 32 + 31, (w / 4) * 32) = reduction;
    }
    if (cnt == 0) {
        //        std::cout << "compress: " << res_hash;
    }

    s_mix[1].range(255, 0) = res_hash;
    mix_hash = res_hash;

    // 4. final Keccak hash
    result = sha3_256_96(s_mix[0], s_mix[1]);
    if (cnt == 0) {
        //        std::cout << "sha256: " << result;
    }
}

inline int tvdiff(struct timeval* tv0, struct timeval* tv1) {
    return (tv1->tv_sec - tv0->tv_sec) * 1000000 + (tv1->tv_usec - tv0->tv_usec);
}

class ArgParser {
   public:
    ArgParser(int& argc, const char** argv) {
        for (int i = 1; i < argc; ++i) mTokens.push_back(std::string(argv[i]));
    }
    bool getCmdOption(const std::string option, std::string& value) const {
        std::vector<std::string>::const_iterator itr;
        itr = std::find(this->mTokens.begin(), this->mTokens.end(), option);
        if (itr != this->mTokens.end() && ++itr != this->mTokens.end()) {
            value = *itr;
            return true;
        }
        return false;
    }

   private:
    std::vector<std::string> mTokens;
};

template <typename T>
T* aligned_alloc(std::size_t num) {
    void* ptr = nullptr;
    if (posix_memalign(&ptr, 4096, num * sizeof(T))) throw std::bad_alloc();
    return reinterpret_cast<T*>(ptr);
}

int main(int argc, char* argv[]) {
    ArgParser parser(argc, (const char**)argv);
    std::string xclbin_path;
#ifndef HLS_TEST
    if (!parser.getCmdOption("-xclbin", xclbin_path)) {
        std::cout << "ERROR:xclbin path is not set!\n";
        return 1;
    }
#endif

    /*    std::string resultfile_path;
        if (!parser.getCmdOption("-resultfile", resultfile_path)) {
            std::cout << "ERROR:dagfile path is not set!\n";
            return 1;
        }*/

    std::string dagfile_path;
    if (!parser.getCmdOption("-dagfile", dagfile_path)) {
        std::cout << "ERROR:dagfile path is not set!\n";
        return 1;
    }

    std::string cnt_str;
    if (!parser.getCmdOption("-n", cnt_str)) {
        std::cout << "ERROR:batch number is not set!\n";
        return 1;
    }
    unsigned int num = std::stoi(cnt_str);

    // prepare host buffer
    // dag size, header hash, nonce_start, nonce_count
    long full_size = 8UL * 1024UL * 1024UL * 1024UL - 4096UL - 4096UL; // 1024 * 1024 * 256 for dag 256MByte test;
    char header_hash_string[] = "~~~X~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    ap_uint<256> header_hash = 0;
    for (int i = 0; i < 32; i++) {
        header_hash.range(i * 8 + 7, i * 8) = header_hash_string[i];
    }
    ap_uint<64> nonce_start = 0x7c7c597c;
    nonce_start = nonce_start;
    ap_uint<64> dag_magic_number;

    ap_uint<512>* full_node = aligned_alloc<ap_uint<512> >((full_size + 63) / 64);

    std::ifstream in;
    in.open(dagfile_path.c_str(), std::ios::in | std::ios::binary);
    if (!in.is_open()) {
        std::cout << "fail to open dag file" << std::endl;
        exit(0);
    } else {
        in.read((char*)&dag_magic_number, 8);
        std::cout << "dag magic number:" << std::hex << dag_magic_number << std::dec << std::endl;
        for (int i = 0; i < ((full_size + 63) / 64); i++) {
            in.read((char*)&full_node[i], 64);
        }
    }
    in.close();

    std::vector<char*> dram(4);
    for (unsigned long int i = 0; i < 3; i++) {
        dram[i] = aligned_alloc<char>(2UL * 1024UL * 1024UL * 1024UL);
    }
    dram[3] = aligned_alloc<char>(2UL * 1024UL * 1024UL * 1024UL - 4096UL - 4096UL);

    ap_uint<1024>* fullnode_1K = reinterpret_cast<ap_uint<1024>*>(full_node);
    std::vector<ap_uint<1024>*> dram1K(4);
    for (int i = 0; i < 4; i++) {
        dram1K[i] = reinterpret_cast<ap_uint<1024>*>(dram[i]);
    }

    for (ap_uint<26> i = 0; i < (full_size + 127) / 128; i++) {
        ap_uint<12> index_remapL = i.range(11, 0);
        ap_uint<2> index_remapM = i.range(13, 12);
        ap_uint<12> index_remapH = i.range(25, 14);
        ap_uint<12> index_remapHR = index_remapH.reverse();

        ap_uint<2> addrH = index_remapM;
        ap_uint<24> addrL = index_remapHR.concat(index_remapL);

        dram1K[addrH][addrL] = fullnode_1K[i];
    }

    char* ret = aligned_alloc<char>(4096UL);
    ap_uint<32>* ret32 = reinterpret_cast<ap_uint<32>*>(ret);

    std::cout << "host map buffer has been allocated" << std::endl;

    ap_uint<256> boundary = 0;

#ifndef HLS_TEST

    // Get CL devices
    std::vector<cl::Device> devices = xcl::get_xil_devices();
    cl::Device device = devices[1];

    // Create context and command queue for selected device
    cl::Context context(device);
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
    std::string devName = device.getInfo<CL_DEVICE_NAME>();
    std::cout << "Selected Device " << devName << "\n";

    cl::Program::Binaries xclBins = xcl::import_binary_file(xclbin_path);
    devices.resize(1);
    devices[0] = device;
    cl::Program program(context, devices, xclBins);
    cl::Kernel kernel0(program, "ethash_kernel0");
    cl::Kernel kernel1(program, "ethash_kernel1");
    cl::Kernel kernel2(program, "ethash_kernel2");
    cl::Kernel kernel3(program, "ethash_kernel3");
    cl::Kernel kernel4(program, "ethash_kernel4");
    cl::Kernel kernel5(program, "ethash_kernel5");
    cl::Kernel kernel6(program, "ethash_kernel6");
    cl::Kernel kernel7(program, "ethash_kernel7");

    std::cout << "Kernel has been created" << std::endl;

    cl_mem_ext_ptr_t mext_dram0 = {(unsigned int)(0) | XCL_MEM_TOPOLOGY, dram[0]};
    cl_mem_ext_ptr_t mext_dram1 = {(unsigned int)(28) | XCL_MEM_TOPOLOGY, dram[1]};
    cl_mem_ext_ptr_t mext_dram2 = {(unsigned int)(23) | XCL_MEM_TOPOLOGY, dram[2]};
    cl_mem_ext_ptr_t mext_dram3 = {(unsigned int)(14) | XCL_MEM_TOPOLOGY, dram[3]};
    cl_mem_ext_ptr_t mext_ret = {(unsigned int)(14) | XCL_MEM_TOPOLOGY, ret};

    // Map buffers
    cl::Buffer buf_dram0(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                         (size_t)(2UL * 1024UL * 1024UL * 1024UL * sizeof(char)), &mext_dram0);
    cl::Buffer buf_dram1(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                         (size_t)(2UL * 1024UL * 1024UL * 1024UL * sizeof(char)), &mext_dram1);
    cl::Buffer buf_dram2(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                         (size_t)(2UL * 1024UL * 1024UL * 1024UL * sizeof(char)), &mext_dram2);
    cl::Buffer buf_dram3(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                         (size_t)((2UL * 1024UL * 1024UL * 1024UL - 4096UL - 4096UL) * sizeof(char)), &mext_dram3);

    cl::Buffer buf_ret(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, 4096UL * sizeof(char),
                       &mext_ret);

    std::cout << "DDR buffers have been mapped" << std::endl;
    //
    struct timeval start_time, end_time;
    gettimeofday(&start_time, 0);

    std::vector<cl::Memory> init;
    init.push_back(buf_dram0);
    init.push_back(buf_dram1);
    init.push_back(buf_dram2);
    init.push_back(buf_dram3);
    init.push_back(buf_ret);
    q.enqueueMigrateMemObjects(init, CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED, nullptr, nullptr);
    q.finish();

    std::vector<cl::Event> write_event;
    std::vector<cl::Event> kernel_event;
    std::vector<cl::Event> read_event;
    write_event.resize(1);
    kernel_event.resize(8);
    read_event.resize(1);

    std::vector<cl::Memory> ib;
    ib.push_back(buf_dram0);
    ib.push_back(buf_dram1);
    ib.push_back(buf_dram2);
    ib.push_back(buf_dram3);
    ib.push_back(buf_ret);
    q.enqueueMigrateMemObjects(ib, 0, nullptr, &write_event[0]);

    unsigned header_hash0 = header_hash.range(31, 0);
    unsigned header_hash1 = header_hash.range(63, 32);
    unsigned header_hash2 = header_hash.range(95, 64);
    unsigned header_hash3 = header_hash.range(127, 96);
    unsigned header_hash4 = header_hash.range(159, 128);
    unsigned header_hash5 = header_hash.range(191, 160);
    unsigned header_hash6 = header_hash.range(223, 192);
    unsigned header_hash7 = header_hash.range(255, 224);

    unsigned boundary0 = boundary.range(31, 0);
    unsigned boundary1 = boundary.range(63, 32);
    unsigned boundary2 = boundary.range(95, 64);
    unsigned boundary3 = boundary.range(127, 96);
    unsigned boundary4 = boundary.range(159, 128);
    unsigned boundary5 = boundary.range(191, 160);
    unsigned boundary6 = boundary.range(223, 192);
    unsigned boundary7 = boundary.range(255, 224);

    unsigned nonce_start0 = nonce_start(31, 0);
    unsigned nonce_start1 = nonce_start(63, 32);
    int j = 0;
    kernel0.setArg(j++, (unsigned)(full_size / 128));
    kernel0.setArg(j++, header_hash0);
    kernel0.setArg(j++, header_hash1);
    kernel0.setArg(j++, header_hash2);
    kernel0.setArg(j++, header_hash3);
    kernel0.setArg(j++, header_hash4);
    kernel0.setArg(j++, header_hash5);
    kernel0.setArg(j++, header_hash6);
    kernel0.setArg(j++, header_hash7);
    kernel0.setArg(j++, boundary0);
    kernel0.setArg(j++, boundary1);
    kernel0.setArg(j++, boundary2);
    kernel0.setArg(j++, boundary3);
    kernel0.setArg(j++, boundary4);
    kernel0.setArg(j++, boundary5);
    kernel0.setArg(j++, boundary6);
    kernel0.setArg(j++, boundary7);
    kernel0.setArg(j++, nonce_start0);
    kernel0.setArg(j++, nonce_start1);
    kernel0.setArg(j++, num);
    kernel0.setArg(j++, buf_dram0);
    kernel0.setArg(j++, buf_dram1);
    kernel0.setArg(j++, buf_dram2);
    kernel0.setArg(j++, buf_dram3);
    kernel0.setArg(j++, buf_ret);

    nonce_start = nonce_start + 4096 * num;
    nonce_start0 = nonce_start(31, 0);
    nonce_start1 = nonce_start(63, 32);
    j = 0;
    kernel1.setArg(j++, (unsigned)(full_size / 128));
    kernel1.setArg(j++, header_hash0);
    kernel1.setArg(j++, header_hash1);
    kernel1.setArg(j++, header_hash2);
    kernel1.setArg(j++, header_hash3);
    kernel1.setArg(j++, header_hash4);
    kernel1.setArg(j++, header_hash5);
    kernel1.setArg(j++, header_hash6);
    kernel1.setArg(j++, header_hash7);
    kernel1.setArg(j++, boundary0);
    kernel1.setArg(j++, boundary1);
    kernel1.setArg(j++, boundary2);
    kernel1.setArg(j++, boundary3);
    kernel1.setArg(j++, boundary4);
    kernel1.setArg(j++, boundary5);
    kernel1.setArg(j++, boundary6);
    kernel1.setArg(j++, boundary7);
    kernel1.setArg(j++, nonce_start0);
    kernel1.setArg(j++, nonce_start1);
    kernel1.setArg(j++, num);
    kernel1.setArg(j++, buf_dram0);
    kernel1.setArg(j++, buf_dram1);
    kernel1.setArg(j++, buf_dram2);
    kernel1.setArg(j++, buf_dram3);
    kernel1.setArg(j++, buf_ret);

    nonce_start = nonce_start + 4096 * num * 2;
    nonce_start0 = nonce_start(31, 0);
    nonce_start1 = nonce_start(63, 32);
    j = 0;
    kernel2.setArg(j++, (unsigned)(full_size / 128));
    kernel2.setArg(j++, header_hash0);
    kernel2.setArg(j++, header_hash1);
    kernel2.setArg(j++, header_hash2);
    kernel2.setArg(j++, header_hash3);
    kernel2.setArg(j++, header_hash4);
    kernel2.setArg(j++, header_hash5);
    kernel2.setArg(j++, header_hash6);
    kernel2.setArg(j++, header_hash7);
    kernel2.setArg(j++, boundary0);
    kernel2.setArg(j++, boundary1);
    kernel2.setArg(j++, boundary2);
    kernel2.setArg(j++, boundary3);
    kernel2.setArg(j++, boundary4);
    kernel2.setArg(j++, boundary5);
    kernel2.setArg(j++, boundary6);
    kernel2.setArg(j++, boundary7);
    kernel2.setArg(j++, nonce_start0);
    kernel2.setArg(j++, nonce_start1);
    kernel2.setArg(j++, num);
    kernel2.setArg(j++, buf_dram0);
    kernel2.setArg(j++, buf_dram1);
    kernel2.setArg(j++, buf_dram2);
    kernel2.setArg(j++, buf_dram3);
    kernel2.setArg(j++, buf_ret);

    nonce_start = nonce_start + 4096 * num * 3;
    nonce_start0 = nonce_start(31, 0);
    nonce_start1 = nonce_start(63, 32);
    j = 0;

    kernel3.setArg(j++, (unsigned)(full_size / 128));
    kernel3.setArg(j++, header_hash0);
    kernel3.setArg(j++, header_hash1);
    kernel3.setArg(j++, header_hash2);
    kernel3.setArg(j++, header_hash3);
    kernel3.setArg(j++, header_hash4);
    kernel3.setArg(j++, header_hash5);
    kernel3.setArg(j++, header_hash6);
    kernel3.setArg(j++, header_hash7);
    kernel3.setArg(j++, boundary0);
    kernel3.setArg(j++, boundary1);
    kernel3.setArg(j++, boundary2);
    kernel3.setArg(j++, boundary3);
    kernel3.setArg(j++, boundary4);
    kernel3.setArg(j++, boundary5);
    kernel3.setArg(j++, boundary6);
    kernel3.setArg(j++, boundary7);
    kernel3.setArg(j++, nonce_start0);
    kernel3.setArg(j++, nonce_start1);
    kernel3.setArg(j++, num);
    kernel3.setArg(j++, buf_dram0);
    kernel3.setArg(j++, buf_dram1);
    kernel3.setArg(j++, buf_dram2);
    kernel3.setArg(j++, buf_dram3);
    kernel3.setArg(j++, buf_ret);

    nonce_start = nonce_start + 4096 * num * 4;
    nonce_start0 = nonce_start(31, 0);
    nonce_start1 = nonce_start(63, 32);
    j = 0;

    kernel4.setArg(j++, (unsigned)(full_size / 128));
    kernel4.setArg(j++, header_hash0);
    kernel4.setArg(j++, header_hash1);
    kernel4.setArg(j++, header_hash2);
    kernel4.setArg(j++, header_hash3);
    kernel4.setArg(j++, header_hash4);
    kernel4.setArg(j++, header_hash5);
    kernel4.setArg(j++, header_hash6);
    kernel4.setArg(j++, header_hash7);
    kernel4.setArg(j++, boundary0);
    kernel4.setArg(j++, boundary1);
    kernel4.setArg(j++, boundary2);
    kernel4.setArg(j++, boundary3);
    kernel4.setArg(j++, boundary4);
    kernel4.setArg(j++, boundary5);
    kernel4.setArg(j++, boundary6);
    kernel4.setArg(j++, boundary7);
    kernel4.setArg(j++, nonce_start0);
    kernel4.setArg(j++, nonce_start1);
    kernel4.setArg(j++, num);
    kernel4.setArg(j++, buf_dram0);
    kernel4.setArg(j++, buf_dram1);
    kernel4.setArg(j++, buf_dram2);
    kernel4.setArg(j++, buf_dram3);
    kernel4.setArg(j++, buf_ret);

    nonce_start = nonce_start + 4096 * num * 5;
    nonce_start0 = nonce_start(31, 0);
    nonce_start1 = nonce_start(63, 32);
    j = 0;

    kernel5.setArg(j++, (unsigned)(full_size / 128));
    kernel5.setArg(j++, header_hash0);
    kernel5.setArg(j++, header_hash1);
    kernel5.setArg(j++, header_hash2);
    kernel5.setArg(j++, header_hash3);
    kernel5.setArg(j++, header_hash4);
    kernel5.setArg(j++, header_hash5);
    kernel5.setArg(j++, header_hash6);
    kernel5.setArg(j++, header_hash7);
    kernel5.setArg(j++, boundary0);
    kernel5.setArg(j++, boundary1);
    kernel5.setArg(j++, boundary2);
    kernel5.setArg(j++, boundary3);
    kernel5.setArg(j++, boundary4);
    kernel5.setArg(j++, boundary5);
    kernel5.setArg(j++, boundary6);
    kernel5.setArg(j++, boundary7);
    kernel5.setArg(j++, nonce_start0);
    kernel5.setArg(j++, nonce_start1);
    kernel5.setArg(j++, num);
    kernel5.setArg(j++, buf_dram0);
    kernel5.setArg(j++, buf_dram1);
    kernel5.setArg(j++, buf_dram2);
    kernel5.setArg(j++, buf_dram3);
    kernel5.setArg(j++, buf_ret);

    nonce_start = nonce_start + 4096 * num * 6;
    nonce_start0 = nonce_start(31, 0);
    nonce_start1 = nonce_start(63, 32);
    j = 0;

    kernel6.setArg(j++, (unsigned)(full_size / 128));
    kernel6.setArg(j++, header_hash0);
    kernel6.setArg(j++, header_hash1);
    kernel6.setArg(j++, header_hash2);
    kernel6.setArg(j++, header_hash3);
    kernel6.setArg(j++, header_hash4);
    kernel6.setArg(j++, header_hash5);
    kernel6.setArg(j++, header_hash6);
    kernel6.setArg(j++, header_hash7);
    kernel6.setArg(j++, boundary0);
    kernel6.setArg(j++, boundary1);
    kernel6.setArg(j++, boundary2);
    kernel6.setArg(j++, boundary3);
    kernel6.setArg(j++, boundary4);
    kernel6.setArg(j++, boundary5);
    kernel6.setArg(j++, boundary6);
    kernel6.setArg(j++, boundary7);
    kernel6.setArg(j++, nonce_start0);
    kernel6.setArg(j++, nonce_start1);
    kernel6.setArg(j++, num);
    kernel6.setArg(j++, buf_dram0);
    kernel6.setArg(j++, buf_dram1);
    kernel6.setArg(j++, buf_dram2);
    kernel6.setArg(j++, buf_dram3);
    kernel6.setArg(j++, buf_ret);

    nonce_start = nonce_start + 4096 * num * 7;
    nonce_start0 = nonce_start(31, 0);
    nonce_start1 = nonce_start(63, 32);
    j = 0;

    kernel7.setArg(j++, (unsigned)(full_size / 128));
    kernel7.setArg(j++, header_hash0);
    kernel7.setArg(j++, header_hash1);
    kernel7.setArg(j++, header_hash2);
    kernel7.setArg(j++, header_hash3);
    kernel7.setArg(j++, header_hash4);
    kernel7.setArg(j++, header_hash5);
    kernel7.setArg(j++, header_hash6);
    kernel7.setArg(j++, header_hash7);
    kernel7.setArg(j++, boundary0);
    kernel7.setArg(j++, boundary1);
    kernel7.setArg(j++, boundary2);
    kernel7.setArg(j++, boundary3);
    kernel7.setArg(j++, boundary4);
    kernel7.setArg(j++, boundary5);
    kernel7.setArg(j++, boundary6);
    kernel7.setArg(j++, boundary7);
    kernel7.setArg(j++, nonce_start0);
    kernel7.setArg(j++, nonce_start1);
    kernel7.setArg(j++, num);
    kernel7.setArg(j++, buf_dram0);
    kernel7.setArg(j++, buf_dram1);
    kernel7.setArg(j++, buf_dram2);
    kernel7.setArg(j++, buf_dram3);
    kernel7.setArg(j++, buf_ret);

    q.enqueueTask(kernel0, &write_event, &kernel_event[0]);
    q.enqueueTask(kernel1, &write_event, &kernel_event[1]);
    q.enqueueTask(kernel2, &write_event, &kernel_event[2]);
    q.enqueueTask(kernel3, &write_event, &kernel_event[3]);
    q.enqueueTask(kernel4, &write_event, &kernel_event[4]);
    q.enqueueTask(kernel5, &write_event, &kernel_event[5]);
    q.enqueueTask(kernel6, &write_event, &kernel_event[6]);
    q.enqueueTask(kernel7, &write_event, &kernel_event[7]);

    std::vector<cl::Memory> ob;
    ob.push_back(buf_ret);
    q.enqueueMigrateMemObjects(ob, CL_MIGRATE_MEM_OBJECT_HOST, &kernel_event, &read_event[0]);
    std::cout << "host kernel call" << std::endl;
    q.finish();
    gettimeofday(&end_time, 0);

    std::cout << "Total execution time " << tvdiff(&start_time, &end_time) / 1000 << "ms" << std::endl;

    unsigned long time1, time2;
    write_event[0].getProfilingInfo(CL_PROFILING_COMMAND_START, &time1);
    write_event[0].getProfilingInfo(CL_PROFILING_COMMAND_END, &time2);
    std::cout << "Write DDR Execution time " << (time2 - time1) / 1000000.0 << "ms" << std::endl;

    kernel_event[0].getProfilingInfo(CL_PROFILING_COMMAND_START, &time1);
    kernel_event[0].getProfilingInfo(CL_PROFILING_COMMAND_END, &time2);
    std::cout << "Kernel0 Execution time " << (time2 - time1) / 1000000.0 << "ms" << std::endl;

    kernel_event[1].getProfilingInfo(CL_PROFILING_COMMAND_START, &time1);
    kernel_event[1].getProfilingInfo(CL_PROFILING_COMMAND_END, &time2);
    std::cout << "Kernel1 Execution time " << (time2 - time1) / 1000000.0 << "ms" << std::endl;

    kernel_event[2].getProfilingInfo(CL_PROFILING_COMMAND_START, &time1);
    kernel_event[2].getProfilingInfo(CL_PROFILING_COMMAND_END, &time2);
    std::cout << "Kernel2 Execution time " << (time2 - time1) / 1000000.0 << "ms" << std::endl;

    kernel_event[3].getProfilingInfo(CL_PROFILING_COMMAND_START, &time1);
    kernel_event[3].getProfilingInfo(CL_PROFILING_COMMAND_END, &time2);
    std::cout << "Kernel3 Execution time " << (time2 - time1) / 1000000.0 << "ms" << std::endl;

    kernel_event[4].getProfilingInfo(CL_PROFILING_COMMAND_START, &time1);
    kernel_event[4].getProfilingInfo(CL_PROFILING_COMMAND_END, &time2);
    std::cout << "Kernel4 Execution time " << (time2 - time1) / 1000000.0 << "ms" << std::endl;

    kernel_event[5].getProfilingInfo(CL_PROFILING_COMMAND_START, &time1);
    kernel_event[5].getProfilingInfo(CL_PROFILING_COMMAND_END, &time2);
    std::cout << "Kernel5 Execution time " << (time2 - time1) / 1000000.0 << "ms" << std::endl;

    kernel_event[6].getProfilingInfo(CL_PROFILING_COMMAND_START, &time1);
    kernel_event[6].getProfilingInfo(CL_PROFILING_COMMAND_END, &time2);
    std::cout << "Kernel6 Execution time " << (time2 - time1) / 1000000.0 << "ms" << std::endl;

    kernel_event[7].getProfilingInfo(CL_PROFILING_COMMAND_START, &time1);
    kernel_event[7].getProfilingInfo(CL_PROFILING_COMMAND_END, &time2);
    std::cout << "Kernel7 Execution time " << (time2 - time1) / 1000000.0 << "ms" << std::endl;

    read_event[0].getProfilingInfo(CL_PROFILING_COMMAND_START, &time1);
    read_event[0].getProfilingInfo(CL_PROFILING_COMMAND_END, &time2);
    std::cout << "Read DDR Execution time " << (time2 - time1) / 1000000.0 << "ms" << std::endl;

#else
    std::vector<ap_uint<512>*> dram512(4);
    for (int i = 0; i < 4; i++) {
        dram512[i] = reinterpret_cast<ap_uint<512>*>(dram[i]);
    }

    ethash_kernel0((full_size / 128), header_hash.range(31, 0), header_hash.range(63, 32), header_hash.range(95, 64),
                   header_hash.range(127, 96), header_hash.range(159, 128), header_hash.range(191, 160),
                   header_hash.range(223, 192), header_hash.range(255, 224), boundary.range(31, 0),
                   boundary.range(63, 32), boundary.range(95, 64), boundary.range(127, 96), boundary.range(159, 128),
                   boundary.range(191, 160), boundary.range(223, 192), boundary.range(255, 224),
                   nonce_start.range(31, 0), nonce_start(63, 32), num, dram512[0], dram512[1], dram512[2], dram512[3],
                   ret32);
#endif
    std::cout << "============================================================" << std::endl;

    ap_uint<32> nonce[8];
    ap_uint<256> mixhash[8];
    ap_uint<256> result[8];

    for (int i = 0; i < 8; i++) {
        nonce[i] = ret32[i * 17];

        mixhash[i].range(31, 0) = ret32[i * 17 + 1];
        mixhash[i].range(63, 32) = ret32[i * 17 + 2];
        mixhash[i].range(95, 64) = ret32[i * 17 + 3];
        mixhash[i].range(127, 96) = ret32[i * 17 + 4];
        mixhash[i].range(159, 128) = ret32[i * 17 + 5];
        mixhash[i].range(191, 160) = ret32[i * 17 + 6];
        mixhash[i].range(223, 192) = ret32[i * 17 + 7];
        mixhash[i].range(255, 224) = ret32[i * 17 + 8];

        result[i].range(31, 0) = ret32[i * 17 + 9];
        result[i].range(63, 32) = ret32[i * 17 + 10];
        result[i].range(95, 64) = ret32[i * 17 + 11];
        result[i].range(127, 96) = ret32[i * 17 + 12];
        result[i].range(159, 128) = ret32[i * 17 + 13];
        result[i].range(191, 160) = ret32[i * 17 + 14];
        result[i].range(223, 192) = ret32[i * 17 + 15];
        result[i].range(255, 224) = ret32[i * 17 + 16];

        std::cout << "---------------------- kernel " << i << " ------------------------" << std::endl;
        std::cout << "nonce: " << nonce[i] << std::endl;
        std::cout << "mixhash: " << std::hex << mixhash[i] << std::dec << std::endl;
        std::cout << "result: " << std::hex << result[i] << std::dec << std::endl;
    }

    /*
        ap_uint<256> tmp_result;
        ap_uint<256> tmp_mix_hash;
        ap_uint<32> sum = 0;
        ap_uint<512>* ret512 = aligned_alloc<ap_uint<512> >(8192UL);
        for (ap_uint<64> i = 0; i < 8192 && i < num * 4096; i++) {
            ap_uint<64> nonce = nonce_start + i;
            ethash_naive(i, tmp_result, tmp_mix_hash, full_node, full_size, header_hash, nonce);
            ret512[i].range(255, 0) = tmp_result;
            ret512[i].range(511, 256) = tmp_mix_hash;
            sum = sum + tmp_result.range(31, 0);
        }
        std::cout << sum << std::endl;
    */
    /*    std::ofstream out;
        out.open(resultfile_path.c_str());

        out << std::hex << std::nouppercase;

        for (unsigned int i = 0; i < 8192 && i < num * 4096; i++) {
            for (int j = 0; j < 32; j++) {
                out << std::setfill('0') << std::setw(2) << ret512[i].range((j + 1) * 8 - 1, j * 8).to_uint();
            }
            out << std::endl;
        }
        std::cout << std::dec << std::endl;
        std::cout << "validate file generate finish" << std::endl;
        free(full_node);
        free(ret);
        out.close();*/

    return 0;
}
