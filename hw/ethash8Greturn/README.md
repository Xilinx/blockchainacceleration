# The Ethash kernel design
   <img src="../design.PNG" width="100%" height="100%">

# RAMA
   The RAMA is called in the postSysLink.tcl
   
   It is include by the Makefile line 119
   
   LDCLFLAGS += --optimize 2 --jobs 16 --advanced.param compiler.userPostSysLinkOverlayTcl=$(CUR_DIR)/postSysLink.tcl
