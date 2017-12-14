TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = svdrc
# svdrc.dbd will be created and installed
DBD += svdrc.dbd

# svdrc.dbd will be made up from these files:
svdrc_DBD += base.dbd

# Include dbd files from all support applications:
#svdrc_DBD += xxx.dbd

# Add all the support libraries needed by this IOC
#svdrc_LIBS += xxx

# svdrc_registerRecordDeviceDriver.cpp derives from svdrc.dbd
svdrc_SRCS += svdrc_registerRecordDeviceDriver.cpp
svdrc_SRCS += svdrc_callback.cpp
svdrc_SRCS += SVDRCCallback.cpp

# Build the main IOC entry point on workstation OSs.
svdrc_SRCS_DEFAULT += svdrcMain.cpp
svdrc_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#svdrc_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
svdrc_LIBS += $(EPICS_BASE_IOC_LIBS)

daq_slc_base_DIR = $(BELLE2_DAQ_SLC)/lib
pq_DIR = $(PGSQL_LIB_PATH)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

USR_INCLUDES += -I$(BELLE2_DAQ_SLC)/include
USR_LIBS += daq_slc_base daq_slc_system daq_slc_database daq_slc_nsm daq_slc_runcontrol daq_slc_psql
USR_LIBS += nsm2_corelib nsm2_b2lib
USR_LIBS += nsm2_corelib nsm2_b2lib
USR_LIBS += m pthread dl z rt pq readline ncurses
