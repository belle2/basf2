TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = rcsample
# rcsample.dbd will be created and installed
DBD += rcsample.dbd

# rcsample.dbd will be made up from these files:
rcsample_DBD += base.dbd

# Include dbd files from all support applications:
rcsample_DBD += rcsample.dbd

# Add all the support libraries needed by this IOC
rcsample_LIBS += seq pv

# rcsample_registerRecordDeviceDriver.cpp derives from rcsample.dbd
rcsample_SRCS += rcsample_registerRecordDeviceDriver.cpp
rcsample_SRCS += rcsample.stt

# Build the main IOC entry point on workstation OSs.
rcsample_SRCS_DEFAULT += rcsampleMain.cpp
rcsample_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#rcsample_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
rcsample_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

