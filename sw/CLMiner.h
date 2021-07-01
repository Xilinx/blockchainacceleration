/// OpenCL miner implementation.
///
/// @file
/// @copyright GNU General Public License

#pragma once

#include <fstream>

#include <libdevcore/Worker.h>
#include <libethcore/EthashAux.h>
#include <libethcore/Miner.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>

#pragma GCC diagnostic push
#if __GNUC__ >= 6
#pragma GCC diagnostic ignored "-Wignored-attributes"
#endif
#pragma GCC diagnostic ignored "-Wmissing-braces"
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS true
#define CL_HPP_ENABLE_EXCEPTIONS true
#define CL_HPP_CL_1_2_DEFAULT_BUILD true
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#include <libethash-cl/xcl2.hpp>
#pragma GCC diagnostic pop

// macOS OpenCL fix:
#ifndef CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV
#define CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV 0x4000
#endif

#ifndef CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV
#define CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV 0x4001
#endif

#include<iostream>
#include<fstream>

namespace dev
{
namespace eth
{

const size_t c_maxSearchResults = 4;
// NOTE: The following struct must match the one defined in
// ethash.cl
struct SearchResults
{
    struct
    {
        uint32_t gid;
        // Can't use h256 data type here since h256 contains
        // more than raw data. Kernel returns raw mix hash.
        uint32_t mix[8];
        uint32_t pad[7];  // pad to 16 words for easy indexing
    } rslt[c_maxSearchResults];
    uint32_t count;
    uint32_t hashCount;
    uint32_t abort;
};

class CLMiner : public Miner
{
public:

    CLMiner(unsigned _index, CLSettings _settings, std::map<std::string, DeviceDescriptor>& _device_description);
    ~CLMiner() override;

    static void enumDevices(std::map<string, DeviceDescriptor>& _DevicesCollection);

protected:
    bool initDevice() override;

    bool initEpoch_internal() override;

    void kick_miner() override;

    void printCL() 
    {
        std::cout << "m_context.size() " << m_context.size() << std::endl;
        std::cout << "m_queue.size() " << m_queue.size() << std::endl;
        std::cout << "m_program.size() " << m_program.size() << std::endl;
        std::cout << "m_searchKernel.size() " << m_searchKernel.size() << std::endl;
        std::cout << "m_ethash_dag_nodes.size() " << m_ethash_dag_nodes.size() << std::endl;
        std::cout << "m_ethash_ret.size() " << m_ethash_ret.size() << std::endl;
    }
private:
    static const int hbm_group_num = 4; // 4 = 8GB / 2GB
    static const int channel_num = 16;
    static const int kernel_enable_num = 8;
    const int single_kernel_batch_num = 128; // at most 8
    static const int total_batch_try = kernel_enable_num * 128 * 4096; // 4096 is from kernel;

    bool dag_checked;
    ofstream f_out_header;
    ofstream f_out_noncestart;
    ofstream f_out_boundary;
    ofstream f_out_nonce;
    ofstream f_out_mix;
    ofstream f_out_res;
 
    void workLoop() override;

    void prepareEthash();

    void updateEthashConfig(const WorkPackage& wp);

    void ethashSearch(SearchResults& result);

    // Device
    int m_device_num;
    vector<DeviceDescriptor> m_device_description;
    vector<cl::Device> m_device;

    // Context, CommandQueue
    vector<cl::Context> m_context;
    vector<cl::CommandQueue> m_queue;

    // Program and Kernel
    vector<cl::Program::Binaries> m_binary;
    vector<cl::Program> m_program;
    vector<vector<cl::Kernel> > m_searchKernel;

    // Buffer
    vector<vector<cl::Buffer> > m_ethash_dag_nodes;
    vector<cl::Buffer> m_ethash_ret;

    // Host Memory
    void* host_dag;
    void* host_dag_remap[hbm_group_num];
    vector<void*> host_ret;

    void clear_buffer() {
        for(int i = 0; i < m_ethash_dag_nodes.size(); i++) {
            m_ethash_dag_nodes[i].clear();
        }
        m_ethash_dag_nodes.clear();
        m_ethash_ret.clear();
    }

    void clear_obj() {
        for(int i = 0; i < m_searchKernel.size(); i++) {
            m_searchKernel[i].clear();
        }
        m_searchKernel.clear();
        m_program.clear();
        m_binary.clear();
    }

    void clear_cq() {
        m_context.clear();
        m_queue.clear();
    }

    CLSettings m_settings;

    unsigned m_dagItems = 0;
    uint64_t m_lastNonce = 0;
};

}  // namespace eth
}  // namespace dev
