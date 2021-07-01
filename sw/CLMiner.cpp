/// OpenCL miner implementation.
///
/// @file
/// @copyright GNU General Public License

#include <boost/dll.hpp>

#include <libethcore/Farm.h>
#include <ethash/ethash.hpp>

#include "CLMiner.h"
#include "ethash.h"

#include "xcl2.hpp"

#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "ap_int.h"

using namespace dev;
using namespace eth;

namespace dev
{
namespace eth
{

// WARNING: Do not change the value of the following constant
// unless you are prepared to make the neccessary adjustments
// to the assembly code for the binary kernels.

struct CLChannel : public LogChannel
{
    static const char* name() { return EthOrange "cl"; }
    static const int verbosity = 2;
    static const bool debug = false;
};
#define cllog clog(CLChannel)

/**
 * Returns the name of a numerical cl_int error
 * Takes constants from CL/cl.h and returns them in a readable format
 */
static const char* strClError(cl_int err)
{
    switch (err)
    {
    case CL_SUCCESS:
        return "CL_SUCCESS";
    case CL_DEVICE_NOT_FOUND:
        return "CL_DEVICE_NOT_FOUND";
    case CL_DEVICE_NOT_AVAILABLE:
        return "CL_DEVICE_NOT_AVAILABLE";
    case CL_COMPILER_NOT_AVAILABLE:
        return "CL_COMPILER_NOT_AVAILABLE";
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:
        return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case CL_OUT_OF_RESOURCES:
        return "CL_OUT_OF_RESOURCES";
    case CL_OUT_OF_HOST_MEMORY:
        return "CL_OUT_OF_HOST_MEMORY";
    case CL_PROFILING_INFO_NOT_AVAILABLE:
        return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case CL_MEM_COPY_OVERLAP:
        return "CL_MEM_COPY_OVERLAP";
    case CL_IMAGE_FORMAT_MISMATCH:
        return "CL_IMAGE_FORMAT_MISMATCH";
    case CL_IMAGE_FORMAT_NOT_SUPPORTED:
        return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case CL_BUILD_PROGRAM_FAILURE:
        return "CL_BUILD_PROGRAM_FAILURE";
    case CL_MAP_FAILURE:
        return "CL_MAP_FAILURE";
    case CL_MISALIGNED_SUB_BUFFER_OFFSET:
        return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
        return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";

#ifdef CL_VERSION_1_2
    case CL_COMPILE_PROGRAM_FAILURE:
        return "CL_COMPILE_PROGRAM_FAILURE";
    case CL_LINKER_NOT_AVAILABLE:
        return "CL_LINKER_NOT_AVAILABLE";
    case CL_LINK_PROGRAM_FAILURE:
        return "CL_LINK_PROGRAM_FAILURE";
    case CL_DEVICE_PARTITION_FAILED:
        return "CL_DEVICE_PARTITION_FAILED";
    case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
        return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
#endif  // CL_VERSION_1_2

    case CL_INVALID_VALUE:
        return "CL_INVALID_VALUE";
    case CL_INVALID_DEVICE_TYPE:
        return "CL_INVALID_DEVICE_TYPE";
    case CL_INVALID_PLATFORM:
        return "CL_INVALID_PLATFORM";
    case CL_INVALID_DEVICE:
        return "CL_INVALID_DEVICE";
    case CL_INVALID_CONTEXT:
        return "CL_INVALID_CONTEXT";
    case CL_INVALID_QUEUE_PROPERTIES:
        return "CL_INVALID_QUEUE_PROPERTIES";
    case CL_INVALID_COMMAND_QUEUE:
        return "CL_INVALID_COMMAND_QUEUE";
    case CL_INVALID_HOST_PTR:
        return "CL_INVALID_HOST_PTR";
    case CL_INVALID_MEM_OBJECT:
        return "CL_INVALID_MEM_OBJECT";
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
        return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case CL_INVALID_IMAGE_SIZE:
        return "CL_INVALID_IMAGE_SIZE";
    case CL_INVALID_SAMPLER:
        return "CL_INVALID_SAMPLER";
    case CL_INVALID_BINARY:
        return "CL_INVALID_BINARY";
    case CL_INVALID_BUILD_OPTIONS:
        return "CL_INVALID_BUILD_OPTIONS";
    case CL_INVALID_PROGRAM:
        return "CL_INVALID_PROGRAM";
    case CL_INVALID_PROGRAM_EXECUTABLE:
        return "CL_INVALID_PROGRAM_EXECUTABLE";
    case CL_INVALID_KERNEL_NAME:
        return "CL_INVALID_KERNEL_NAME";
    case CL_INVALID_KERNEL_DEFINITION:
        return "CL_INVALID_KERNEL_DEFINITION";
    case CL_INVALID_KERNEL:
        return "CL_INVALID_KERNEL";
    case CL_INVALID_ARG_INDEX:
        return "CL_INVALID_ARG_INDEX";
    case CL_INVALID_ARG_VALUE:
        return "CL_INVALID_ARG_VALUE";
    case CL_INVALID_ARG_SIZE:
        return "CL_INVALID_ARG_SIZE";
    case CL_INVALID_KERNEL_ARGS:
        return "CL_INVALID_KERNEL_ARGS";
    case CL_INVALID_WORK_DIMENSION:
        return "CL_INVALID_WORK_DIMENSION";
    case CL_INVALID_WORK_GROUP_SIZE:
        return "CL_INVALID_WORK_GROUP_SIZE";
    case CL_INVALID_WORK_ITEM_SIZE:
        return "CL_INVALID_WORK_ITEM_SIZE";
    case CL_INVALID_GLOBAL_OFFSET:
        return "CL_INVALID_GLOBAL_OFFSET";
    case CL_INVALID_EVENT_WAIT_LIST:
        return "CL_INVALID_EVENT_WAIT_LIST";
    case CL_INVALID_EVENT:
        return "CL_INVALID_EVENT";
    case CL_INVALID_OPERATION:
        return "CL_INVALID_OPERATION";
    case CL_INVALID_GL_OBJECT:
        return "CL_INVALID_GL_OBJECT";
    case CL_INVALID_BUFFER_SIZE:
        return "CL_INVALID_BUFFER_SIZE";
    case CL_INVALID_MIP_LEVEL:
        return "CL_INVALID_MIP_LEVEL";
    case CL_INVALID_GLOBAL_WORK_SIZE:
        return "CL_INVALID_GLOBAL_WORK_SIZE";
    case CL_INVALID_PROPERTY:
        return "CL_INVALID_PROPERTY";

#ifdef CL_VERSION_1_2
    case CL_INVALID_IMAGE_DESCRIPTOR:
        return "CL_INVALID_IMAGE_DESCRIPTOR";
    case CL_INVALID_COMPILER_OPTIONS:
        return "CL_INVALID_COMPILER_OPTIONS";
    case CL_INVALID_LINKER_OPTIONS:
        return "CL_INVALID_LINKER_OPTIONS";
    case CL_INVALID_DEVICE_PARTITION_COUNT:
        return "CL_INVALID_DEVICE_PARTITION_COUNT";
#endif  // CL_VERSION_1_2

#ifdef CL_VERSION_2_0
    case CL_INVALID_PIPE_SIZE:
        return "CL_INVALID_PIPE_SIZE";
    case CL_INVALID_DEVICE_QUEUE:
        return "CL_INVALID_DEVICE_QUEUE";
#endif  // CL_VERSION_2_0

#ifdef CL_VERSION_2_2
    case CL_INVALID_SPEC_ID:
        return "CL_INVALID_SPEC_ID";
    case CL_MAX_SIZE_RESTRICTION_EXCEEDED:
        return "CL_MAX_SIZE_RESTRICTION_EXCEEDED";
#endif  // CL_VERSION_2_2
    }

    return "Unknown CL error encountered";
}

/**
 * Prints cl::Errors in a uniform way
 * @param msg text prepending the error message
 * @param clerr cl:Error object
 *
 * Prints errors in the format:
 *      msg: what(), string err() (numeric err())
 */
static std::string ethCLErrorHelper(const char* msg, cl::Error const& clerr)
{
    std::ostringstream osstream;
    osstream << msg << ": " << clerr.what() << ": " << strClError(clerr.err()) << " ("
             << clerr.err() << ")";
    return osstream.str();
}

namespace
{
void addDefinition(string& _source, char const* _id, unsigned _value)
{
    char buf[256];
    sprintf(buf, "#define %s %uu\n", _id, _value);
    _source.insert(_source.begin(), buf, buf + strlen(buf));
}

std::vector<cl::Platform> getPlatforms()
{
    vector<cl::Platform> platforms;
    try
    {
        cl::Platform::get(&platforms);
    }
    catch (cl::Error const& err)
    {
#if defined(CL_PLATFORM_NOT_FOUND_KHR)
        if (err.err() == CL_PLATFORM_NOT_FOUND_KHR)
            std::cerr << "No OpenCL platforms found" << std::endl;
        else
#endif
            std::cerr << "OpenCL error : " << err.what();
    }
    return platforms;
}

std::vector<cl::Device> getDevices(
    std::vector<cl::Platform> const& _platforms, unsigned _platformId)
{
    vector<cl::Device> devices;
    size_t platform_num = min<size_t>(_platformId, _platforms.size() - 1);
    try
    {
        _platforms[platform_num].getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);
    }
    catch (cl::Error const& err)
    {
        // if simply no devices found return empty vector
        if (err.err() != CL_DEVICE_NOT_FOUND)
            throw err;
    }
    return devices;
}

template <typename T>
T* aligned_alloc(std::size_t num) {
    void* ptr = nullptr;
    if (posix_memalign(&ptr, 4096, num * sizeof(T))) throw std::bad_alloc();
    return reinterpret_cast<T*>(ptr);
}

}  // namespace

}  // namespace eth
}  // namespace dev

void CLMiner::prepareEthash() {
    if(m_context.size() == 0 || m_queue.size() == 0 || m_binary.size() == 0 || m_program.size() == 0 || m_searchKernel.size() == 0 || m_ethash_dag_nodes.size() == 0 || m_ethash_ret.size() == 0) {
        clear_buffer();
        clear_obj();
        clear_cq();

        // context and command queue
        m_context.push_back(cl::Context(m_device));
        for(int i = 0; i < m_device_num; i++) {
            m_queue.push_back(cl::CommandQueue(m_context[0], m_device[i], CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE));
        } 

        int sys_call_ret;
        //sys_call_ret=system("xbutil program -p ethash_kernel.xclbin");
        //sys_call_ret=system("xbutil clock -f 200 -h 300");

        // binary and program
        std::string ethash_xclbin_path("./ethash_kernel.xclbin");
        m_binary.push_back(xcl::import_binary_file(ethash_xclbin_path));
        for(int i = 0; i < m_device_num; i++) {
            m_program.push_back(cl::Program(m_context[0], {m_device[i]}, m_binary[0]));
        }

        // kernel
        m_searchKernel.resize(m_device_num);
        for(int i = 0; i < m_device_num; i++) {
            m_searchKernel[i].push_back(cl::Kernel(m_program[i], "ethash_kernel0"));
            m_searchKernel[i].push_back(cl::Kernel(m_program[i], "ethash_kernel1"));
            m_searchKernel[i].push_back(cl::Kernel(m_program[i], "ethash_kernel2"));
            m_searchKernel[i].push_back(cl::Kernel(m_program[i], "ethash_kernel3"));
            m_searchKernel[i].push_back(cl::Kernel(m_program[i], "ethash_kernel4"));
            m_searchKernel[i].push_back(cl::Kernel(m_program[i], "ethash_kernel5"));
            m_searchKernel[i].push_back(cl::Kernel(m_program[i], "ethash_kernel6"));
            m_searchKernel[i].push_back(cl::Kernel(m_program[i], "ethash_kernel7"));
        }

        // buffer
        m_ethash_dag_nodes.resize(m_device_num);
        m_ethash_ret.resize(m_device_num);
        for(int d = 0; d < m_device_num; d++) {
            cl_mem_ext_ptr_t mext_act_dag_nodes[hbm_group_num];
            for (int i = 0; i < hbm_group_num; i++) {
                mext_act_dag_nodes[i] = {i + 20, host_dag_remap[i], m_searchKernel[d][0]()};
            }
            cl_mem_ext_ptr_t mext_act_ret = {24, host_ret[d], m_searchKernel[d][0]()};

            m_ethash_dag_nodes[d].resize(hbm_group_num);
            for (int i = 0; i < hbm_group_num; i++) {
                m_ethash_dag_nodes[d][i] = cl::Buffer(m_context[0], CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(char) * (2048UL * 1024UL * 1024UL - 4096UL), &mext_act_dag_nodes[i]);
            }
            m_ethash_ret[d] = cl::Buffer(m_context[0], CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(char) * (4096UL), &mext_act_ret);
        }

        // transfer DAG
/*
        std::cout << "DAG transfer begin" << std::endl;
        vector<vector<cl::Memory> > ib;
        ib.resize(m_device_num);
        for(int d = 0; d < m_device_num; d++) {
            for (int i = 0; i < hbm_group_num; i++) {
                ib[d].push_back(m_ethash_dag_nodes[d][i]);
            }
            ib[d].push_back(m_ethash_ret[d]);
        }

        vector<cl::Event> update_dag_event;
        update_dag_event.resize(m_device_num);
        for(int d = 0; d < m_device_num; d++) {
            m_queue[d].enqueueMigrateMemObjects(ib[d], 0, nullptr, &update_dag_event[d]);
            m_queue[d].finish();
            std::cout << "DAG for device[" << d << "] is done" << std::endl;
        }
        std::cout << "DAG transfer finished" << std::endl;
*/
        for(int d = 0; d < m_device_num; d++) {
            vector<cl::Memory> ib;
            for(int i = 0; i < hbm_group_num; i++) {
                ib.push_back(m_ethash_dag_nodes[d][i]);
            }
            ib.push_back(m_ethash_ret[d]);

            cl::Event update_dag_event;
            m_queue[d].enqueueMigrateMemObjects(ib, 0, nullptr, &update_dag_event);
            m_queue[d].finish();
            std::cout << "device[" << d << "]'s DAG is finished" << std::endl;
        }
    }
}

void CLMiner::updateEthashConfig(const WorkPackage& wp) {
    // act dag size
    uint32_t full_size = m_epochContext.dagSize / 128;

    // header
    ap_uint<256> header = 0;
    memcpy((void*)&header, wp.header.data(), (256 / 8));
    uint32_t header_32[8];
    for(int i = 0; i < 8; i++) {
        header_32[i] = header.range(31 + i * 32, i * 32);
    }

    // boundary
    ap_uint<256> boundary = 0;
    for(int i = 0; i < 256 / 8; i++) {
        memcpy(((void*)&boundary) + i, &(wp.boundary.data()[256 / 8 - 1 - i]), 1);
    }
    uint32_t boundary_32[8];
    for(int i = 0; i < 8; i++) {
        boundary_32[i] = boundary.range(31 + i * 32, i * 32);
    }    

    for(int d = 0; d < m_device_num; d++) {    
        // nonce_start
        uint32_t nonce_start0[8];
        uint32_t nonce_start1[8];
        for(int i = 0; i < 8; i++) {
            ap_uint<64> nonce_start = wp.startNonce + (i + d * kernel_enable_num) * single_kernel_batch_num * 4096;
            nonce_start0[i] = nonce_start.range(31, 0); 
            nonce_start1[i] = nonce_start.range(63, 32);
        }

        // set arguments
        for(int i = 0; i < kernel_enable_num; i++) {
            int j = 0;
            m_searchKernel[d][i].setArg(j++, full_size);
            m_searchKernel[d][i].setArg(j++, header_32[0]);
            m_searchKernel[d][i].setArg(j++, header_32[1]);
            m_searchKernel[d][i].setArg(j++, header_32[2]);
            m_searchKernel[d][i].setArg(j++, header_32[3]);
            m_searchKernel[d][i].setArg(j++, header_32[4]);
            m_searchKernel[d][i].setArg(j++, header_32[5]);
            m_searchKernel[d][i].setArg(j++, header_32[6]);
            m_searchKernel[d][i].setArg(j++, header_32[7]);
            m_searchKernel[d][i].setArg(j++, boundary_32[0]);
            m_searchKernel[d][i].setArg(j++, boundary_32[1]);
            m_searchKernel[d][i].setArg(j++, boundary_32[2]);
            m_searchKernel[d][i].setArg(j++, boundary_32[3]);
            m_searchKernel[d][i].setArg(j++, boundary_32[4]);
            m_searchKernel[d][i].setArg(j++, boundary_32[5]);
            m_searchKernel[d][i].setArg(j++, boundary_32[6]);
            m_searchKernel[d][i].setArg(j++, boundary_32[7]);
            m_searchKernel[d][i].setArg(j++, nonce_start0[i]);
            m_searchKernel[d][i].setArg(j++, nonce_start1[i]);
            m_searchKernel[d][i].setArg(j++, single_kernel_batch_num);
            m_searchKernel[d][i].setArg(j++, m_ethash_dag_nodes[d][0]);
            m_searchKernel[d][i].setArg(j++, m_ethash_dag_nodes[d][1]);
            m_searchKernel[d][i].setArg(j++, m_ethash_dag_nodes[d][2]);
            m_searchKernel[d][i].setArg(j++, m_ethash_dag_nodes[d][3]);
            m_searchKernel[d][i].setArg(j++, m_ethash_ret[d]);
        }
    }
}

void CLMiner::ethashSearch(SearchResults& result) {
    vector<vector<cl::Event> > task_event;
    task_event.resize(m_device_num);
    for(int d = 0; d < m_device_num; d++) {
        task_event[d].resize(kernel_enable_num);
        for(int i = 0; i < kernel_enable_num; i++) {
            m_queue[d].enqueueTask(m_searchKernel[d][i], nullptr, &task_event[d][i]);
        }
    }

    vector<vector<cl::Memory> > ob;
    ob.resize(m_device_num);
    for(int d = 0; d < m_device_num; d++) {
        ob[d].push_back(m_ethash_ret[d]);
    }

    vector<cl::Event> read_event;
    read_event.resize(m_device_num);
    for(int d = 0; d < m_device_num; d++) {
        m_queue[d].enqueueMigrateMemObjects(ob[d], CL_MIGRATE_MEM_OBJECT_HOST, &task_event[d], &read_event[d]);
    }

    for(int d = 0; d < m_device_num; d++) {
        m_queue[d].finish();
    }

    result.count = 0;
    for(int d = 0; d < m_device_num; d++) {
        for(int i = 0; i < kernel_enable_num; i++) {
            uint32_t nonce_tmp = ((uint32_t*)host_ret[d])[i * 17];
            if(nonce_tmp < (1 << 31)) {// MSB is 0 when valid nonce is founded
                if(result.count < c_maxSearchResults) {// keep at most the first c_maxSearchResult sucess result
                    // gid
                    // ethash will get nonce by nonce = nonce start + gid
                    // since 8 kernels has different nonce start, as real nonce start = nonce start + i * single_kernel_batch_num * 4096
                    // here we will add the "i * single_kernel_batch_num * 4096" to gid  
                    result.rslt[result.count].gid = ((uint32_t*)host_ret[d])[i * 17] + (i + d * kernel_enable_num) * single_kernel_batch_num * 4096;
                    // mix
                    memcpy(result.rslt[result.count].mix, host_ret[d] + 17 * 4 * i + 4, (256 / 8));
                    // update result.count
                    result.count++;
                    std::cout << "One qualified result from device[" << d << "]" << std::endl;
                }
            }
        }
    }

    if(result.count != 0) {
        std::cout << "One batch search, got " << result.count << " qualified result" << std::endl;
        std::cout << "actual result count = " << result.count << std::endl;
    }
}

CLMiner::CLMiner(unsigned _index, CLSettings _settings, std::map<std::string, DeviceDescriptor>& _device_description)
  : Miner("cl-", _index), m_settings(_settings)
{
    m_settings.localWorkSize = ((m_settings.localWorkSize + 7) / 8) * 8;
    m_settings.globalWorkSize = m_settings.localWorkSize * m_settings.globalWorkSizeMultiplier;

    for(auto it = _device_description.begin(); it != _device_description.end(); it++) {
        m_device_description.push_back(it->second);
    }
    m_device_num = m_device_description.size();

    dag_checked = false;

    host_dag = aligned_alloc<char>(1024UL * 1024UL * 1024UL * 8UL);
    host_ret.resize(m_device_num);
    for(int i = 0; i < m_device_num; i++) {
        host_ret[i] = aligned_alloc<char>(4096UL);
        uint32_t* ret_32 = reinterpret_cast<uint32_t*>(host_ret[i]);
        for(int i = 0; i < 8 * 17; i++) {
            ret_32[i] = 0;
        }
    }
    
    for(int i = 0; i < hbm_group_num; i++) {
        host_dag_remap[i] = aligned_alloc<char>(1024UL * 1024UL * 1024UL * 2UL - 4096UL);
    }

    f_out_header.open("f_out_header.dat", ios::out | ios::binary);
    f_out_noncestart.open("f_out_noncestart.dat", ios::out | ios::binary);
    f_out_boundary.open("f_out_boundary.dat", ios::out | ios::binary);
    f_out_nonce.open("f_out_nonce.dat", ios::out | ios::binary);
    f_out_mix.open("f_out_mix.dat", ios::out | ios::binary);
    f_out_res.open("f_out_res.dat", ios::out | ios::binary);
}

CLMiner::~CLMiner()
{
    DEV_BUILD_LOG_PROGRAMFLOW(cllog, "cl-" << m_index << " CLMiner::~CLMiner() begin");
    stopWorking();
    kick_miner();
    free(host_dag);
    for(int i = 0; i < m_device_num; i++) {
        free(host_ret[i]);
    }
    for(int i = 0; i < hbm_group_num; i++) {
        free(host_dag_remap[i]);
    }
    DEV_BUILD_LOG_PROGRAMFLOW(cllog, "cl-" << m_index << " CLMiner::~CLMiner() end");
    f_out_header.close();
    f_out_noncestart.close();
    f_out_boundary.close();
    f_out_nonce.close();
    f_out_mix.close();
    f_out_res.close();
}

void CLMiner::workLoop()
{
    uint64_t startNonce = 0;

    // The work package currently processed by GPU.
    WorkPackage current;
    current.header = h256();

    if (!initDevice())
    return;

    try
    {
        while (!shouldStop())
        {

            // Read results.
            SearchResults results;

            // Wait for work or 3 seconds (whichever the first)
            const WorkPackage w = work();

            //prepareEthash();

            if (!w)
            {
                boost::system_time const timeout =
                    boost::get_system_time() + boost::posix_time::seconds(3);
                boost::mutex::scoped_lock l(x_work);
                m_new_work_signal.timed_wait(l, timeout);
                continue;
            }

            bool fromCurrent = false;

            if (current.header != w.header)
            {
                std::cout << "Header Change" << std::endl;
                if (current.epoch != w.epoch)
                {
                    if (!initEpoch())
                        break;  // This will simply exit the thread
                    prepareEthash();
                }

                // Upper 64 bits of the boundary.
                const uint64_t target = (uint64_t)(u64)((u256)w.boundary >> 192);
                assert(target > 0);

                startNonce = w.startNonce;

                updateEthashConfig(w);
            } else {
                updateEthashConfig(current);
                fromCurrent = true;
            }

            // Run the ethash kernel.
            ethashSearch(results); 
            
            // Report the result
            if (results.count)
            {
                // Report results while the kernel is running.
                for (uint32_t i = 0; i < results.count; i++)
                {
                    uint64_t nonce;
                    if(fromCurrent) {
                        nonce = current.startNonce + results.rslt[i].gid;
                    } else {
                        nonce = w.startNonce + results.rslt[i].gid;
                    }

                    if (nonce != m_lastNonce)
                    {
                        m_lastNonce = nonce;
                        h256 mix;
                        memcpy(mix.data(), (char*)results.rslt[i].mix, sizeof(results.rslt[i].mix));

                        Farm::f().submitProof(Solution{
                            nonce, mix, current, std::chrono::steady_clock::now(), m_index});
                        cllog << EthWhite << "Job: " << current.header.abridged() << " Sol: 0x"
                              << toHex(nonce) << EthReset;

                        const WorkPackage* ptr;
                        if(fromCurrent) {
                            ptr = &current;
                        } else {
                            ptr = &w;
                        }

                        f_out_header.write((char*)( ptr->header.data() ), (256 / 8));
                        f_out_noncestart.write((char*)( &(ptr->startNonce) ), (64 / 8));
                        f_out_boundary.write((char*)( ptr->boundary.data() ), (256 / 8));
                        f_out_nonce.write((char*)( &nonce ), (64 / 8));
                        f_out_mix.write((char*)( mix.data() ), (256 / 8));

                        f_out_header.flush();
                        f_out_noncestart.flush();
                        f_out_boundary.flush();
                        f_out_nonce.flush();
                        f_out_mix.flush();
                        f_out_res.flush();
                    }
                }
            }

            current = w;  // kernel now processing newest work
            current.startNonce = startNonce;
            // Increase start nonce for following kernel execution.
            startNonce += total_batch_try * m_device_num;
            // Report hash count
            updateHashRate(total_batch_try * m_device_num, 1);
        }

        if (m_queue.size()) {
            for(int d = 0; d < m_device_num; d++) {
                m_queue[d].finish();
            }
        }

        clear_buffer();
    }
    catch (cl::Error const& _e)
    {
        string _what = ethCLErrorHelper("OpenCL Error", _e);
        clear_buffer();
        throw std::runtime_error(_what);
    }
}

void CLMiner::kick_miner()
{
    // Memory for abort Cannot be static because crashes on macOS.
    m_new_work_signal.notify_one();
}

void CLMiner::enumDevices(std::map<string, DeviceDescriptor>& _DevicesCollection)
{
    // Load available platforms
    vector<cl::Platform> platforms = getPlatforms();
    if (platforms.empty())
        return;

    unsigned int dIdx = 0;
    for (unsigned int pIdx = 0; pIdx < platforms.size(); pIdx++)
    {
        std::string platformName = platforms.at(pIdx).getInfo<CL_PLATFORM_NAME>();
        ClPlatformTypeEnum platformType = ClPlatformTypeEnum::Unknown;
        if (platformName == "AMD Accelerated Parallel Processing")
            platformType = ClPlatformTypeEnum::Amd;
        else if (platformName == "Clover" || platformName == "Intel Gen OCL Driver")
            platformType = ClPlatformTypeEnum::Clover;
        else if (platformName == "NVIDIA CUDA")
            platformType = ClPlatformTypeEnum::Nvidia;
        else if (platformName.find("Intel") != string::npos)
            platformType = ClPlatformTypeEnum::Intel;
        else if (platformName == "Xilinx")
            platformType = ClPlatformTypeEnum::Xilinx;
        else
        {
            std::cerr << "Unrecognized platform " << platformName << std::endl;
            continue;
        }


        std::string platformVersion = platforms.at(pIdx).getInfo<CL_PLATFORM_VERSION>();
        unsigned int platformVersionMajor = std::stoi(platformVersion.substr(7, 1));
        unsigned int platformVersionMinor = std::stoi(platformVersion.substr(9, 1));

        dIdx = 0;
        vector<cl::Device> devices = getDevices(platforms, pIdx);
        for (auto const& device : devices)
        {
            //std::cout << "Single card support for test" << std::endl;
            //auto const& device = devices[0];

            DeviceTypeEnum clDeviceType = DeviceTypeEnum::Unknown;
            cl_device_type detectedType = device.getInfo<CL_DEVICE_TYPE>();
            if (detectedType == CL_DEVICE_TYPE_GPU)
                clDeviceType = DeviceTypeEnum::Gpu;
            else if (detectedType == CL_DEVICE_TYPE_CPU)
                clDeviceType = DeviceTypeEnum::Cpu;
            else if (detectedType == CL_DEVICE_TYPE_ACCELERATOR)
                clDeviceType = DeviceTypeEnum::Accelerator;

            string uniqueId;
            DeviceDescriptor deviceDescriptor;

            if (clDeviceType == DeviceTypeEnum::Gpu && platformType == ClPlatformTypeEnum::Nvidia)
            {
                cl_int bus_id, slot_id;
                if (clGetDeviceInfo(device.get(), 0x4008, sizeof(bus_id), &bus_id, NULL) ==
                        CL_SUCCESS &&
                    clGetDeviceInfo(device.get(), 0x4009, sizeof(slot_id), &slot_id, NULL) ==
                        CL_SUCCESS)
                {
                    std::ostringstream s;
                    s << setfill('0') << setw(2) << hex << bus_id << ":" << setw(2)
                      << (unsigned int)(slot_id >> 3) << "." << (unsigned int)(slot_id & 0x7);
                    uniqueId = s.str();
                }
            }
            else if (clDeviceType == DeviceTypeEnum::Gpu &&
                     (platformType == ClPlatformTypeEnum::Amd ||
                         platformType == ClPlatformTypeEnum::Clover))
            {
                cl_char t[24];
                if (clGetDeviceInfo(device.get(), 0x4037, sizeof(t), &t, NULL) == CL_SUCCESS)
                {
                    std::ostringstream s;
                    s << setfill('0') << setw(2) << hex << (unsigned int)(t[21]) << ":" << setw(2)
                      << (unsigned int)(t[22]) << "." << (unsigned int)(t[23]);
                    uniqueId = s.str();
                }
            }
            else if (clDeviceType == DeviceTypeEnum::Gpu && platformType == ClPlatformTypeEnum::Intel)
            {
                std::ostringstream s;
                s << "Intel GPU " << pIdx << "." << dIdx;
                uniqueId = s.str();

            }
            else if (clDeviceType == DeviceTypeEnum::Cpu)
            {
                std::ostringstream s;
                s << "CPU:" << setfill('0') << setw(2) << hex << (pIdx + dIdx);
                uniqueId = s.str();
            }
            else if (platformType == ClPlatformTypeEnum::Xilinx)
            {
                std::ostringstream s;
                s << "Xilinx Alveo:" << setfill('0') << setw(2) << hex << (pIdx + dIdx);
                uniqueId = s.str();
            }
            else
            {
                // We're not prepared (yet) to handle other platforms or types
                ++dIdx;
                continue;
            }

           if (_DevicesCollection.find(uniqueId) != _DevicesCollection.end())
                deviceDescriptor = _DevicesCollection[uniqueId];
            else
                deviceDescriptor = DeviceDescriptor();

            // Fill the blanks by OpenCL means
            deviceDescriptor.name = device.getInfo<CL_DEVICE_NAME>();
            deviceDescriptor.type = clDeviceType;
            deviceDescriptor.uniqueId = uniqueId;
            deviceDescriptor.clDetected = true;
            deviceDescriptor.clPlatformId = pIdx;
            deviceDescriptor.clPlatformName = platformName;
            deviceDescriptor.clPlatformType = platformType;
            deviceDescriptor.clPlatformVersion = platformVersion;
            deviceDescriptor.clPlatformVersionMajor = platformVersionMajor;
            deviceDescriptor.clPlatformVersionMinor = platformVersionMinor;
            deviceDescriptor.clDeviceOrdinal = dIdx;

            deviceDescriptor.clName = deviceDescriptor.name;
            deviceDescriptor.clDeviceVersion = device.getInfo<CL_DEVICE_VERSION>();
            deviceDescriptor.clDeviceVersionMajor =
                std::stoi(deviceDescriptor.clDeviceVersion.substr(7, 1));
            deviceDescriptor.clDeviceVersionMinor =
                std::stoi(deviceDescriptor.clDeviceVersion.substr(9, 1));
            deviceDescriptor.totalMemory = device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();
            deviceDescriptor.clMaxMemAlloc = device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
            deviceDescriptor.clMaxWorkGroup = device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
            deviceDescriptor.clMaxComputeUnits = device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();

            // Apparently some 36 CU devices return a bogus 14!!!
            deviceDescriptor.clMaxComputeUnits =
                deviceDescriptor.clMaxComputeUnits == 14 ? 36 : deviceDescriptor.clMaxComputeUnits;

            // Is it an NVIDIA card ?
            if (platformType == ClPlatformTypeEnum::Nvidia)
            {
                size_t siz;
                clGetDeviceInfo(device.get(), CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV,
                    sizeof(deviceDescriptor.clNvComputeMajor), &deviceDescriptor.clNvComputeMajor,
                    &siz);
                clGetDeviceInfo(device.get(), CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV,
                    sizeof(deviceDescriptor.clNvComputeMinor), &deviceDescriptor.clNvComputeMinor,
                    &siz);
                deviceDescriptor.clNvCompute = to_string(deviceDescriptor.clNvComputeMajor) + "." +
                                               to_string(deviceDescriptor.clNvComputeMinor);
            }

            // Upsert Devices Collection
            _DevicesCollection[uniqueId] = deviceDescriptor;
            ++dIdx;

        }
    }
}

bool CLMiner::initDevice()
{

    // LookUp device
    // Load available platforms
    vector<cl::Platform> platforms = getPlatforms();
    if (platforms.empty())
        return false;

    for(int i = 0; i < m_device_num; i++) {
        vector<cl::Device> devices = getDevices(platforms, m_device_description[i].clPlatformId);
        if (devices.empty())
            return false;

        m_device.push_back(devices.at(m_device_description[i].clDeviceOrdinal));

        // Set Hardware Monitor Info
        if (m_device_description[i].clPlatformType == ClPlatformTypeEnum::Xilinx)
        {
            //TBA for Xilinx Platform
        }
        else
        {
            // Don't know what to do with this
            cllog << "Only Xilinx Platform is supported";
            return false;
        }
    }

    if (!m_settings.noExit && (m_hwmoninfo.deviceType != HwMonitorInfoType::AMD))
    {
        m_settings.noExit = true;
    }

    return true;
}

bool CLMiner::initEpoch_internal()
{

    // release ethash search related Context / CommandQueue / Kernel / Buffer
    clear_buffer();
    clear_obj();
    clear_cq();
    std::cout << "clear done " << std::endl;
    //
    auto startInit = std::chrono::steady_clock::now();
    size_t RequiredMemory = (m_epochContext.dagSize);

    // Release the pause flag if any
    resume(MinerPauseEnum::PauseDueToInsufficientMemory);
    resume(MinerPauseEnum::PauseDueToInitEpochError);

    // dag_kernel related parameters
    size_t cache_size = m_epochContext.lightSize;
    size_t cache_nodes_num = cache_size >> 6;

    size_t act_dag_size = m_epochContext.dagSize;
    size_t act_dag_nodes_num = m_epochContext.dagNumItems * 2;
    size_t part_dag_size = 1 << 27;
    size_t part_dag_nodes_count = part_dag_size >> 6;

    const size_t cfgs_num = 3;
    void* cache_nodes = aligned_alloc<char>(cache_size);
    memcpy(cache_nodes, m_epochContext.lightCache, cache_size);
    uint32_t* cfgs = aligned_alloc<uint32_t>(cfgs_num);

    const int max_dag_rounds = 4;
    int act_dag_rounds = (act_dag_size + (1024UL * 1024UL * 1024UL * 2UL - 1)) / (1024UL * 1024UL * 1024UL * 2UL);
    std::cout << "Need " << act_dag_rounds << " rounds to generate DAG dataset of " << act_dag_size << " Bytes." << std::endl;

    size_t dag_start_index = 0;
    cfgs[0] = cache_nodes_num;
    cfgs[1] = dag_start_index;
    cfgs[2] = part_dag_nodes_count;

    // dump cache to file
    ofstream f_cache;
    std::cout << "cache size: " << cache_size << " Byte" << std::endl;
    f_cache.open("f_cache.dat", ios::out | ios::binary);
    f_cache.write((char*)cache_nodes, cache_size);
    f_cache.close();

    // binary / context / commandQueue / kernel

    cl::Context context(m_device[0]);
    cl::CommandQueue queue(context, m_device[0], CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
    std::string dag_xclbin_path("./dag_gen_kernel.xclbin");
    cl::Program::Binaries xclBins = xcl::import_binary_file(dag_xclbin_path);
    cl::Program program(context, {m_device[0]}, xclBins);
    cl::Kernel dag_gen_kernel(program, "dag_gen_kernel");

    // Buffer
    cl_mem_ext_ptr_t mext_cache_nodes[channel_num];
    cl_mem_ext_ptr_t mext_dag_nodes[channel_num];
    cl_mem_ext_ptr_t mext_cfgs = {channel_num * 2, cfgs, dag_gen_kernel()};
    for (int i = 0; i < channel_num; i++) {
        mext_cache_nodes[i] = {i, cache_nodes, dag_gen_kernel()};
        mext_dag_nodes[i] = {i + channel_num, host_dag + (i + channel_num * (max_dag_rounds - 1)) * part_dag_size, dag_gen_kernel()};
    }

    vector<cl::Buffer> m_buff_cache_nodes;
    vector<cl::Buffer> m_buff_dag_nodes;
    vector<cl::Buffer> m_buff_cfgs;

    m_buff_cache_nodes.resize(channel_num);
    m_buff_dag_nodes.resize(channel_num);
    m_buff_cfgs.resize(1);
    for (int i = 0; i < channel_num; i++) {
        m_buff_cache_nodes[i] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                                         sizeof(char) * (cache_size), &mext_cache_nodes[i]);
        m_buff_dag_nodes[i] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                                       sizeof(char) * (part_dag_size), &mext_dag_nodes[i]);
    }
    m_buff_cfgs[0] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                           sizeof(uint32_t) * (cfgs_num), &mext_cfgs);
    // read/write/kernel events
    std::vector<cl::Event> write_event[max_dag_rounds];
    std::vector<cl::Event> kernel_event[max_dag_rounds];
    std::vector<cl::Event> read_event[max_dag_rounds];
    for(int i = 0; i < max_dag_rounds; i++) {
        write_event[i].resize(1);
        kernel_event[i].resize(1);
        read_event[i].resize(1);
    }
    // ib cb and ob
    std::vector<cl::Memory> ib;
    ib.push_back(m_buff_cfgs[0]);
    for (int i = 0; i < channel_num; i++) {
        ib.push_back(m_buff_cache_nodes[i]);
        ib.push_back(m_buff_dag_nodes[i]);
    }

    std::vector<cl::Memory> cb;
    cb.push_back(m_buff_cfgs[0]);

    std::vector<cl::Memory> ob;
    for (int i = 0; i < channel_num; i++) {
        ob.push_back(m_buff_dag_nodes[i]);
    }

    // set kernel arguments
    for (int i = 0; i < channel_num; i++) {
        dag_gen_kernel.setArg(i, m_buff_cache_nodes[i]);
    }
    for (int i = 0; i < channel_num; i++) {
        dag_gen_kernel.setArg(i + channel_num, m_buff_dag_nodes[i]);
    }
    dag_gen_kernel.setArg(channel_num * 2, m_buff_cfgs[0]);

    // Generate 2GB DAG each time, memcpy to correct location
    for(int i = 0; i < act_dag_rounds; i++) {
        dag_start_index = part_dag_nodes_count * channel_num * i;
        cfgs[1] = dag_start_index;

        if(i == 0) { // first call need to transfer cache, dag and config
            queue.enqueueMigrateMemObjects(ib, 0, NULL, &write_event[i][0]);
        } else { // other than first call, only need to update config
            queue.enqueueMigrateMemObjects(cb, 0, NULL, &write_event[i][0]);
        }

        queue.enqueueTask(dag_gen_kernel, &write_event[i], &kernel_event[i][0]);
        queue.enqueueMigrateMemObjects(ob, CL_MIGRATE_MEM_OBJECT_HOST, &kernel_event[i], &read_event[i][0]);
        queue.finish();

        if(i != (max_dag_rounds - 1)) { // need to mem copy to correct location
            memcpy(host_dag + part_dag_size * channel_num * i, host_dag + part_dag_size * channel_num * (max_dag_rounds - 1), part_dag_size * channel_num);
        }
    }
        // remap dag
    ap_uint<1024>* fullnode_1K = reinterpret_cast<ap_uint<1024>*>(host_dag);
    ap_uint<1024>* dram1K[4];
    for (int i = 0; i < 4; i++) {
        dram1K[i] = reinterpret_cast<ap_uint<1024>*>(host_dag_remap[i]);
    }
    for (ap_uint<26> i = 0; i < (act_dag_size + 127) / 128; i++) {
        ap_uint<12> index_remapL = i.range(11, 0);
        ap_uint<2> index_remapM = i.range(13, 12);
        ap_uint<12> index_remapH = i.range(25, 14);
        ap_uint<12> index_remapHR = index_remapH.reverse();

        ap_uint<2> addrH = index_remapM;
        ap_uint<24> addrL = index_remapHR.concat(index_remapL);

        dram1K[addrH][addrL] = fullnode_1K[i];
    }

    // dump dag to file
    ofstream f_dag;
    std::cout << "dag size: " << act_dag_size << " Byte" << std::endl;
    f_dag.open("f_dag.dat", ios::out | ios::binary);
    f_dag.write((char*)host_dag, act_dag_size);

    // clear dag gen related
    clear_buffer();
    clear_obj();
    clear_cq();

    // re-create ethash search related
    //
    free(cache_nodes);
    free(cfgs);

    return true;
}

