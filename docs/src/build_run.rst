.. 
      Copyright 2021 Xilinx, Inc.
  
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
  
       http://www.apache.org/licenses/LICENSE-2.0
  
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

.. meta::
   :keywords: Vitis, Alveo, Ethereum, Mining
   :description: Steps to build and run
   :xlnxdocumentclass: Document
   :xlnxdocumenttype: Tutorials


Steps To Build
==============

We need to get **host** and **xclbin** files ready before run mining. 

Build Host
----------

To build ethminer host:

.. code-block:: bash

    source /opt/xilinx/xrt/setup.sh
    ./build.sh build_host


Download or Build Xclbin Files
------------------------------

We need two xclbin files to run mining: **dag_gen_kernel.xclbin** and **ethash_kernel.xclbin**.
They could be downloaded from Xilinx website or built with Vitis.
We recommend downloading them because building them locally might takes 10+ hours.

To download xclbins from Xilinx:

.. code-block:: bash

    ./build.sh get_xclbin

To build xclbins locally:

.. code-block:: bash

    ./build.sh build_daggen
    ./build.sh build_ethash

How to run mining
================

To run mining online, we need to prepare a wallet address, a identifier for your miner and mining pool address.

.. code-block:: bash

    ./build.sh mine <wallet address> <identifier name> <mining pool address>

