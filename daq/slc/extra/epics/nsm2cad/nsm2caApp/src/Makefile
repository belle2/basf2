TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = nsm2cad
# nsm2cad.dbd will be created and installed
DBD += nsm2cad.dbd

# nsm2cad.dbd will be made up from these files:
nsm2cad_DBD += base.dbd

# Include dbd files from all support applications:
nsm2cad_DBD += deviceNSM2CA.dbd
nsm2cad_DBD += ioc_shell.dbd

# Add all the support libraries needed by this IOC
#nsm2cad_LIBS += xxx

# nsm2cad_registerRecordDeviceDriver.cpp derives from nsm2cad.dbd
nsm2cad_SRCS += nsm2cad_registerRecordDeviceDriver.cpp
nsm2cad_SRCS += NSM2CACallback.cpp
nsm2cad_SRCS += NSM2CA.cpp
nsm2cad_SRCS += devNSM2CA.cpp
nsm2cad_SRCS += deviceNSM2CA.c
nsm2cad_SRCS += ioc_shell.c

# Build the main IOC entry point on workstation OSs.
nsm2cad_SRCS_DEFAULT += nsm2cadMain.cpp
nsm2cad_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#nsm2cad_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
nsm2cad_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

daq_slc_base_DIR = $(BELLE2_DAQ_SLC)/lib
pq_DIR = $(PGSQL_LIB_PATH)

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

USR_INCLUDES += -I$(BELLE2_DAQ_SLC)/include
USR_LIBS += daq_slc_base daq_slc_system daq_slc_database daq_slc_nsm daq_slc_runcontrol daq_slc_hvcontrol daq_slc_psql
USR_LIBS += nsm2_corelib nsm2_b2lib
USR_LIBS += nsm2_corelib nsm2_b2lib
USR_LIBS += m pthread dl z rt pq readline ncurses 

