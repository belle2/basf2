#include "daq/slc/hvcontrol/arichlv/ArichlvControlCallback.h"
#include "daq/slc/system/LogFile.h"
#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/DBObjectLoader.h>
#include <daq/slc/system/PThread.h>

#include <cstdlib>
#include "daq/slc/hvcontrol/arichlv/MPOD.h"

using namespace Belle2;
#define MPODCH(slot,channel) ((slot)*100+(channel))


/*
  Setting parameters from database at beginings of process
*/
void ArichlvControlCallback::initialize(const HVConfig& config) throw()
{
  MPOD_Start();
  LogFile::debug("Initialize %s:%d ", __FILE__, __LINE__);
  int retval = MPOD_Open(0, "f9mpod2.ijs.si");
  if (retval != 0) LogFile::error("%s:%d ", __FILE__, __LINE__);
  retval = MPOD_Open(1, "f9mpod.ijs.si");
  if (retval != 0) LogFile::error("%s:%d ", __FILE__, __LINE__);

  LogFile::info("%s:%d Initialized", __FILE__, __LINE__);
  try {
    load(config, false, true);
  } catch (const HVHandlerException& e) {
    LogFile::error(e.what());
  }

  LogFile::info("%s:%d ", __FILE__, __LINE__);
}



void ArichlvControlCallback::update() throw(HVHandlerException)
{
  /*performed at the interval of hv.interval*/
}

void ArichlvControlCallback::setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException)
{
  int setval = (switchon == true) ? 1 : 0;
  LogFile::info("%s:%d ", __FILE__, __LINE__);
  setChannelSwitch(crateHsnmp[crate], MPODCH(slot, channel), setval);
  /*set channel switch (true:ON, false:OFF)*/
  LogFile::info("setswitch called : crate = %d, slot = %d, channel = %d, switch: %s",
                crate, slot, channel, (switchon ? "ON" : "OFF"));
}

void ArichlvControlCallback::setRampUpSpeed(int crate, int slot, int channel, float voltage) throw(IOException)
{
  /*set voltage ramp up speed with unit of [V]*/
  setModuleRampSpeedVoltage(crateHsnmp[crate], slot, voltage);
  LogFile::info("setrampup called : crate = %d, slot = %d, channel = %d, voltage: %f ---> Module function common for slot",
                crate, slot, channel, voltage);
}

void ArichlvControlCallback::setRampDownSpeed(int crate, int slot, int channel, float voltage) throw(IOException)
{
  /*set voltage ramp down speed with unit of [V]*/
  LogFile::info("setrampdown called : not implemeted in MPODf");
}

void ArichlvControlCallback::setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException)
{
  /*set demand voltage with unit of [V]*/
  setOutputVoltage(crateHsnmp[crate], MPODCH(slot, channel), voltage);
  LogFile::info("setvoltagedemand called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, voltage);
}

void ArichlvControlCallback::setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException)
{
  /*set voltage limit with unit of [V]*/
  LogFile::info("setvoltagelimit called : not implemeted in MPOD");
}

void ArichlvControlCallback::setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException)
{
  /*set current limit with unit of [uA]*/
  setOutputCurrent(crateHsnmp[crate], MPODCH(slot, channel), current);
  LogFile::info("setcurrentlimit called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, current);
}

float ArichlvControlCallback::getRampUpSpeed(int crate, int slot, int channel) throw(IOException)
{

  float ret = getModuleRampSpeedVoltage(crateHsnmp[crate], slot);
  /*return voltage ramp up speed with unit of [V]*/
  return ret;
}

float ArichlvControlCallback::getRampDownSpeed(int crate, int slot, int channel) throw(IOException)
{

  float ret = getModuleRampSpeedVoltage(crateHsnmp[crate], slot);
  /*return voltage ramp down speed with unit of [V]*/
  return ret;
}

float ArichlvControlCallback::getVoltageDemand(int crate, int slot, int channel) throw(IOException)
{
  float ret = getOutputVoltage(crateHsnmp[crate], MPODCH(slot, channel));
  /*return demand voltage with unit of [V]*/
  return ret;
}

float ArichlvControlCallback::getVoltageLimit(int crate, int slot, int channel) throw(IOException)
{
  float ret = getModuleHardwareLimitVoltage(crateHsnmp[crate], slot);
  /*return voltage limit with unit of [V]*/
  return ret;
}

float ArichlvControlCallback::getCurrentLimit(int crate, int slot, int channel) throw(IOException)
{
  float ret = getModuleHardwareLimitCurrent(crateHsnmp[crate], slot);
  /*return current limit with unit of [uA]*/
  return ret;
}

float ArichlvControlCallback::getVoltageMonitor(int crate, int slot, int channel) throw(IOException)
{
  double  ret = getOutputSenseMeasurement(crateHsnmp[crate], MPODCH(slot, channel));
  /*return monitored voltage with unit of [V]*/
  return ret;
}

float ArichlvControlCallback::getCurrentMonitor(int crate, int slot, int channel) throw(IOException)
{

  double ret = getCurrentMeasurement(crateHsnmp[crate], MPODCH(slot, channel));
  /*return monitored current with unit of [uA]*/
  return ret;
}

bool ArichlvControlCallback::getSwitch(int crate, int slot, int channel) throw(IOException)
{
  int status = getChannelSwitch(crateHsnmp[crate], MPODCH(slot, channel));
  /*return status of channel swicth (true:ON, false:OFF)*/
  return (status) ? true : false;
}

int ArichlvControlCallback::getState(int crate, int slot, int channel) throw(IOException)
{
  /*
    return channel status:
    OFF: power off,
    ON: power on without error
    OCP: trip due to over current
    OVP: trip due to over voltage
    ERR: another error
    RAMPUP: ramping up voltage
    RAMPDOWN: ramping down voltage
  */
  return HVMessage::OFF;
}

