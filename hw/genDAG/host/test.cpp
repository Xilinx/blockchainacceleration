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

#include <sys/time.h>
#include <new>
#include <cstdlib>
#include <ap_int.h>
#include <iostream>
#include "xcl2.hpp"

#include <vector>
#include <string>
#include <fstream>

#include "dag_gen.hpp"

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
    //
    ArgParser parser(argc, (const char**)argv);
    std::string xclbin_path;
    if (!parser.getCmdOption("-xclbin", xclbin_path)) {
        std::cout << "ERROR:xclbin path is not set!\n";
        return 1;
    }

    //
    const int channel_num = 16;
    ap_uint<32> cache_size = 1 << 26;
    std::cout << std::hex << "cache_size:" << cache_size << std::endl;
    ap_uint<32> cache_nodes_num = cache_size >> 6;
    std::cout << std::hex << "cache_nodes_num:" << cache_nodes_num << std::endl;
    ap_uint<32> dag_size = 1 << 27;
    std::cout << std::hex << "single channel dag_size:" << dag_size << std::endl;
    ap_uint<32> dag_nodes_count = dag_size >> 6;
    std::cout << std::hex << "dag_nodes_count:" << dag_nodes_count << std::endl;
    ap_uint<32> dag_start_index = 0;
    const ap_uint<32> cfgs_num = 3;
    const ap_uint<256> seed;
    const char seed_char[33] = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    for (int i = 0; i < 32; i++) {
        seed.range(i * 8 + 7, i * 8) = seed_char[i];
    }

    //
    uint64_t full_dag_size = 1;
    full_dag_size <<= 32;
    uint64_t full_dag_nodes_num = full_dag_size >> 6;
    std::cout << "full dag size:" << full_dag_size << std::endl;
    ap_uint<512>* golden_dag = aligned_alloc<ap_uint<512> >(full_dag_size >> 6);
    if (golden_dag == NULL) {
        std::cout << "fail to allocate golden_dag" << std::endl;
        return 1;
    } else {
        std::cout << "succeed to allocate golden_dag" << std::endl;
    }

    ap_uint<512>* res_dag = aligned_alloc<ap_uint<512> >(full_dag_size >> 6);
    if (res_dag == NULL) {
        std::cout << "fail to allocate res_dag" << std::endl;
        return 1;
    } else {
        std::cout << "succeed to allocate res_dag" << std::endl;
    }

    std::string golden_path;
    if (!parser.getCmdOption("-dagfile", golden_path)) {
        std::cout << "ERROR:golden dag file path is not set!\n";
        return 1;
    } else {
        std::ifstream in_dag;
        in_dag.open(golden_path.c_str(), std::ios::in | std::ios::binary);
        if (!in_dag.is_open()) {
            std::cout << "ERROR:faild to open golden dag file!\n" << std::endl;
            return -1;
        } else {
            ap_uint<64> dag_magic_number;
            in_dag.read((char*)(&dag_magic_number), 8);
            std::cout << "dag magic number:" << std::hex << dag_magic_number << std::dec << std::endl;
            for (int i = 0; i < ((full_dag_size + 63) / 64); i++) {
                in_dag.read((char*)&golden_dag[i], 64);
            }
            std::cout << "golden dag loaded" << std::endl;
        }
    }

    //
    ap_uint<512>* cache_nodes = aligned_alloc<ap_uint<512> >(cache_nodes_num);
    std::string cache_path;
    if (!parser.getCmdOption("-cachefile", cache_path)) {
        std::cout << "ERROR:cachefile path is not set!\n";
        return 1;
    } else {
        std::ifstream in_cache;
        in_cache.open(cache_path.c_str(), std::ios::in | std::ios::binary);
        if (!in_cache.is_open()) {
            std::cout << "ERROR:failed ot open cache file!" << std::endl;
        } else {
            for (int i = 0; i < cache_nodes_num; i++) {
                in_cache.read((char*)&cache_nodes[i], 64);
            }
        }
        std::cout << "cache_node ready!" << std::endl;
    }
    //

    ap_uint<32>* cfgs = aligned_alloc<ap_uint<32> >(cfgs_num);
    cfgs[0] = cache_nodes_num;
    cfgs[1] = dag_start_index;
    cfgs[2] = dag_nodes_count;

    std::cout << "host map buffer has been allocated" << std::endl;

    // Get CL devices
    std::vector<cl::Device> devices = xcl::get_xil_devices();
    cl::Device device = devices[0];

    // Create context and command queue for selected device
    cl::Context context(device);
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
    std::string devName = device.getInfo<CL_DEVICE_NAME>();
    std::cout << "Selected Device " << devName << "\n";

    cl::Program::Binaries xclBins = xcl::import_binary_file(xclbin_path);
    devices.resize(1);
    cl::Program program(context, devices, xclBins);
    cl::Kernel kernel(program, "dag_gen_kernel");
    std::cout << "Kernel has been created" << std::endl;

    // Map buffers
    cl_mem_ext_ptr_t mext_cache_nodes[channel_num];
    cl_mem_ext_ptr_t mext_dag_nodes[channel_num];
    cl_mem_ext_ptr_t mext_cfgs = {(unsigned int)(34) | XCL_MEM_TOPOLOGY, cfgs, 0};
    for (int i = 0; i < channel_num; i++) {
        mext_cache_nodes[i] = {(unsigned int)(i) | XCL_MEM_TOPOLOGY, cache_nodes, 0};
        mext_dag_nodes[i] = {(unsigned int)(i) | XCL_MEM_TOPOLOGY, res_dag + (i + channel_num) * dag_nodes_count, 0};
    }

    cl::Buffer buff_cache_nodes[channel_num];
    cl::Buffer buff_dag_nodes[channel_num];
    cl::Buffer buff_cfgs;
    for (int i = 0; i < channel_num; i++) {
        buff_cache_nodes[i] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                                         sizeof(char) * (cache_size), &mext_cache_nodes[i]);
        buff_dag_nodes[i] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                                       sizeof(char) * (dag_size), &mext_dag_nodes[i]);
    }
    buff_cfgs = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                           sizeof(ap_uint<32>) * (cfgs_num), &mext_cfgs);

    std::cout << "HBM buffers have been mapped" << std::endl;

    //
    struct timeval start_time, mid_time, mid_1_time, end_time;
    gettimeofday(&start_time, 0);

    //
    std::vector<cl::Event> write_event[2];
    std::vector<cl::Event> kernel_event[2];
    std::vector<cl::Event> read_event[2];
    write_event[0].resize(1);
    write_event[1].resize(1);
    kernel_event[0].resize(1);
    kernel_event[1].resize(1);
    read_event[0].resize(1);
    read_event[1].resize(1);

    std::vector<cl::Memory> ib;
    ib.push_back(buff_cfgs);
    for (int i = 0; i < channel_num; i++) {
        ib.push_back(buff_cache_nodes[i]);
        ib.push_back(buff_dag_nodes[i]);
    }
    q.enqueueMigrateMemObjects(ib, 0, nullptr, &write_event[0][0]);

    for (int i = 0; i < channel_num; i++) {
        kernel.setArg(i, buff_cache_nodes[i]);
    }
    for (int i = 0; i < channel_num; i++) {
        kernel.setArg(i + channel_num, buff_dag_nodes[i]);
    }
    kernel.setArg(channel_num * 2, buff_cfgs);

    // 1st 2GB dag
    q.enqueueTask(kernel, &write_event[0], &kernel_event[0][0]);

    std::vector<cl::Memory> ob;
    for (int i = 0; i < channel_num; i++) {
        ob.push_back(buff_dag_nodes[i]);
    }
    q.enqueueMigrateMemObjects(ob, CL_MIGRATE_MEM_OBJECT_HOST, &kernel_event[0], &read_event[0][0]);
    q.finish();

    gettimeofday(&mid_time, 0);
    memcpy(res_dag, res_dag + dag_nodes_count * 16, dag_size * 16);

    gettimeofday(&mid_1_time, 0);
    // 2nd 2GB dag
    dag_start_index = dag_nodes_count * 16;
    cfgs[1] = dag_start_index;

    q.enqueueMigrateMemObjects(ib, 0, &write_event[0], &write_event[1][0]);

    q.enqueueTask(kernel, &write_event[1], &kernel_event[1][0]);

    q.enqueueMigrateMemObjects(ob, CL_MIGRATE_MEM_OBJECT_HOST, &kernel_event[1], &read_event[1][0]);
    q.finish();

    gettimeofday(&end_time, 0);
    std::cout << "first 2GB takes:" << std::dec << tvdiff(&start_time, &mid_time) << " us" << std::endl;
    std::cout << "move first 2GB takes:" << std::dec << tvdiff(&mid_time, &mid_1_time) << " us" << std::endl;
    std::cout << "second 2GB takes:" << std::dec << tvdiff(&mid_1_time, &end_time) << " us" << std::endl;
    std::cout << "Total execution time " << std::dec << tvdiff(&start_time, &end_time) << " us" << std::endl;

    int not_match_num = 0;
    for (int i = 0; i < full_dag_nodes_num; i++) {
        if (golden_dag[i] != res_dag[i]) {
            not_match_num++;
        }
    }
    std::cout << "Unmatched items in 4G dag:" << not_match_num << std::endl;

    // clean up
    free(cache_nodes);
    free(cfgs);
    free(golden_dag);
    free(res_dag);

    return not_match_num;
}
