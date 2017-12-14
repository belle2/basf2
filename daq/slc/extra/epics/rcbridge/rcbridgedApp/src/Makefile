TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = rcbridged
# rcbridged.dbd will be created and installed
DBD += rcbridged.dbd

# rcbridged.dbd will be made up from these files:
rcbridged_DBD += base.dbd

# Include dbd files from all support applications:
rcbridged_DBD += deviceRC.dbd
rcbridged_DBD += ioc_shell.dbd

# Add all the support libraries needed by this IOC
#rcbridged_LIBS += xxx

# rcbridged_registerRecordDeviceDriver.cpp derives from rcbridged.dbd
rcbridged_SRCS += rcbridged_registerRecordDeviceDriver.cpp
rcbridged_SRCS += devRC.cpp
rcbridged_SRCS += deviceRC.c
rcbridged_SRCS += ioc_shell.c

# Build the main IOC entry point on workstation OSs.
rcbridged_SRCS_DEFAULT += rcbridgedMain.cpp
rcbridged_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed

# Finally link to the EPICS Base libraries
rcbridged_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

daq_slc_base_DIR = $(BELLE2_DAQ_SLC)/lib
pq_DIR = $(PGSQL_LIB_PATH)/lib

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

USR_INCLUDES += -I$(BELLE2_DAQ_SLC)/include
USR_LIBS += daq_slc_base daq_slc_system daq_slc_database daq_slc_nsm daq_slc_runcontrol
USR_LIBS += nsm2_corelib nsm2_b2lib
USR_LIBS += nsm2_corelib nsm2_b2lib
USR_LIBS += m pthread dl z rt pq readline ncurses 

