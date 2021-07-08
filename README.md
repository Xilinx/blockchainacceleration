# EthereumMiner

EthereumMiner is Xilinx's open-sourced solution written in C++ for Ethereum mining. 

Currently, this solution offers:
* On device side, a Vitis based kernel design to accelerate Ethash, which is hashing algorithm of Ethereum.
* Also, a Vitis based kernel design to accelerate "DAG" generation, which provides the large dataset needed by Ethash.
* On host side, a patch to "ethminer" which is the most popular mining software, for customer to do mining with their own account and desired mining pool.

## Requirement
* Currently this solution support Xilinx Alveo U55N, you need installed packages for the card, like XRT / firmware and development package for the card.
* If you would like to build the xclbin files on your own machine, you'll also need Vitis that is at least 2021.1_released or later version. You could also download pre-buildt xclbin files from Xilinx. Please take reference from "Step to run". 

## Step to run
* 1. Get xclbins ready.
  You can either download them from Xilinx or try build them yourself.
  To download from Xilinx: "./build.sh get_xclbin".
  To build your self: "./build.sh build_ethash" and "./build.sh build_daggen"
* 2. Get ethminer ready: "./build.sh build_host"
* 3. Run: "./build mine <your_account> <your_mine_name> <mining_pool_address>"
  


## License

Licensed using the [Apache 2.0 license](https://www.apache.org/licenses/LICENSE-2.0).

    Copyright 2019-2021 Xilinx, Inc.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
