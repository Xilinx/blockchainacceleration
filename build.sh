#!/bin/bash

# ## License

# Licensed using the [Apache 2.0 license](https://www.apache.org/licenses/LICENSE-2.0).

#     Copyright 2019-2021 Xilinx, Inc.
#     
#     Licensed under the Apache License, Version 2.0 (the "License");
#     you may not use this file except in compliance with the License.
#     You may obtain a copy of the License at
#     
#         http://www.apache.org/licenses/LICENSE-2.0
#     
#     Unless required by applicable law or agreed to in writing, software
#     distributed under the License is distributed on an "AS IS" BASIS,
#     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#     See the License for the specific language governing permissions and
#     limitations under the License.


function print_help() {
    echo ""
    echo "Usage: ./build.sh [option]"
    echo "Options:"
    echo "help                                     print this help"
    echo "build_ethash                             build ethash_kernel.xclbin"
    echo "build_daggen                             build dag_gen_kernel.xclbin"
    echo "build_host                               build ethminer"
    echo "get_xclbin                               down xclbin files needed from Xilinx OMS"
    echo "mine <wallet> <name> <pool_address>      start mining with your wallet, name and pool address"
    echo ""
}

function build_ethash() {
    echo ""
    if [ ! $XILINX_XRT ]
    then
        echo "Please setup XRT"
    elif [ ! $XILINX_VITIS ]
    then
        echo "Please setup Vitis"
    else
        export DEVICE=u55n_gen3x4
        export TARGET=hw
        echo "Start to build ethash_kernel.xclbin, will takes several hours"
        cd ./hw/ethash8Greturn
        make all
        cp build_dir.$TARGET.$DEVICE/ethash_kernel.xclbin ../../
        cd ../..
    fi
    echo ""
}

function build_daggen() {
    echo ""
    if [ ! $XILINX_XRT ]
    then
        echo "Please setup XRT"
    elif [ ! $XILINX_VITIS ]
    then
        echo "Please setup Vitis"
    else
        export DEVICE=u55n_gen3x4
        export TARGET=hw
        echo "Start to build dag_gen_kernel.xclbin, will takes several hours"
        cd ./hw/genDAG
        make all
        cp build_dir.$TARGET.$DEVICE/dag_gen_kernel.xclbin ../../
        cd ../..
    fi
    echo ""
}

function get_xclbin() {
    echo ""
    echo "Download xclbins files needed from Xilinx OMS, not finished yet"
    echo ""
}

function build_host() {
    echo ""
    if [ ! $XILINX_XRT ]
    then
        echo "Please setup XRT"
    else
        echo "Download ethminer"
        git clone https://github.com/ethereum-mining/ethminer.git
        wget https://boostorg.jfrog.io/artifactory/main/release/1.66.0/source/boost_1_66_0.7z
        mv boost_1_66_0.7z ~/.hunter/_Base/Download/Boost/1.66.0/075d0b4/
        cd ethminer
        git checkout -b xilinx_platform
        git reset --hard cd75c13d38eceb6fed78d47104440a762ca1894e
        git config user.email "dummy@dummy.com"
        git config user.name "Dummy Name"
        git am --abort
        git am ../sw/*.patch
        git submodule update --init --recursive
        mkdir build
        cd build
        cmake ..
        make -j 10
        cd ../../
    fi
    echo ""
}

function mine() {
    echo ""
    echo "mine"
    echo "account:              $1"
    echo "name:                 $2"
    echo "mining pool address:  $3"

    xclbin_1="ethash_kernel.xclbin"
    xclbin_2="dag_gen_kernel.xclbin"
    exe_file="ethminer/build/ethminer/ethminer"
    if [ -f $xclbin_1 ]
    then
        cp $xclbin_1 ethminer/build/ethminer/
    else
        echo "Please build $xclbin_1 or download from Xilinx"
    fi
    if [ -f $xclbin_2 ]
    then
        cp $xclbin_2 ethminer/build/ethminer/
    else
        echo "Please build $xclbin_2 or download from Xilinx"
    fi
    if [ -f $exe_file ]
    then
        :
    else
        echo "Please build ethminer"
    fi

    if [ -f $xclbin_1 ] && [ -f $xclbin_2 ] && [ -f $exe_file ]
    then
        $exe_file -P stratum1+tcp://$1.$2@$3 | tee ./test.log
    fi
}


if [ $# == 0 ]
then
    print_help
elif [ $1 = help ]
then
    print_help
elif [ $1 = build_ethash ]
then
    build_ethash
elif [ $1 = build_daggen ]
then
    build_daggen
elif [ $1 = get_xclbin ]
then
    get_xclbin
elif [ $1 = build_host ]
then
    build_host
elif [ $1 = mine ]
then
    if [ $# = 4 ]
    then
        mine $2 $3 $4
    else
        echo ""
        echo "Need provide acount, name and mining pool address"
        print_help
    fi
else
    echo ""
    echo "Unrecognized option"
    print_help
fi
