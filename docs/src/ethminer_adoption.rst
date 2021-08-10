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
   :description: Ethminer Adoption
   :xlnxdocumentclass: Document
   :xlnxdocumenttype: Tutorials


Ethminer Adoption
=================

Ethminer is one of the most popular open-source mining software. We provide a patch to modify ethminer to adopt Xilinx solution. Basically we modify the opencl branch in ethminer. When Epoch nubmer change, it will stop mining and run dag_gen_kernel to generate latest DAG dataset and do necessary remapping for ethash_kernel. When resume mining, it will keep communicate with mining pool and get latest work. Based on the latest work, it will setup cofig for ethash_kernel and run. After ethash_kernel finished, it will check the returned result and commit successful nonce to mining pool.


.. image:: /images/ethminer_adoption.png
   :scale: 60%
   :align: center
