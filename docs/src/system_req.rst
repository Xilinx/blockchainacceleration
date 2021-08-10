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
   :description: System requirement
   :xlnxdocumentclass: Document
   :xlnxdocumenttype: Tutorials


Software Platform
=================

This library is designed to work with Vitis 2021.1 and later, and therefore inherits the system requirements of Vitis and XRT.

Supported operating systems are RHEL/CentOS 7.4 or higher, 18.04.1 LTS.
With CentOS/RHEL, C++11/C++14 should be enabled via
[devtoolset-6](https://www.softwarecollections.org/en/scls/rhscl/devtoolset-6/).

Hardware Platform
=================

In this release, acceleration kernels are designed and optimized to work with Varium C1100.

To run mining Ethereum coin online, it needs at least 16GB free memory to handle necessary DAG dataset.

