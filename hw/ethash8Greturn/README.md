#
# Copyright 2019-2020 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#


# The Ethash kernel design
   <img src="../design.PNG" width="100%" height="100%">

# RAMA
   The RAMA is called in the postSysLink.tcl
   
   It is include by the Makefile line 119
   
   LDCLFLAGS += --optimize 2 --jobs 16 --advanced.param compiler.userPostSysLinkOverlayTcl=$(CUR_DIR)/postSysLink.tcl
