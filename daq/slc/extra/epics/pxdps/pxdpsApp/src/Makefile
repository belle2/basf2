TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = pxdps
# pxdps.dbd will be created and installed
DBD += pxdps.dbd

# pxdps.dbd will be made up from these files:
pxdps_DBD += base.dbd

# Include dbd files from all support applications:
#pxdps_DBD += xxx.dbd

# Add all the support libraries needed by this IOC
#pxdps_LIBS += xxx

# pxdps_registerRecordDeviceDriver.cpp derives from pxdps.dbd
pxdps_SRCS += pxdps_registerRecordDeviceDriver.cpp
pxdps_SRCS += pxdhv_callback.cpp
pxdps_SRCS += PXDHVControlCallback.cpp

# Build the main IOC entry point on workstation OSs.
pxdps_SRCS_DEFAULT += pxdpsMain.cpp
pxdps_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#pxdps_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
pxdps_LIBS += $(EPICS_BASE_IOC_LIBS)

daq_slc_base_DIR = $(BELLE2_DAQ_SLC)/lib
pq_DIR = $(PGSQL_LIB_PATH)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

USR_INCLUDES += -I$(BELLE2_DAQ_SLC)/include
USR_LIBS += daq_slc_base daq_slc_system daq_slc_database daq_slc_nsm daq_slc_hvcontrol daq_slc_runcontrol daq_slc_psql
USR_LIBS += nsm2_corelib nsm2_b2lib
USR_LIBS += nsm2_corelib nsm2_b2lib
USR_LIBS += m pthread dl z rt pq readline ncurses
