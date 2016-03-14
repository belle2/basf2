#include "daq/slc/hvcontrol/template/TemplateHVControlCallback.h"

#include "daq/slc/system/LogFile.h"

#include <cstdlib>

using namespace Belle2;

/*
  Setting parameters from database at beginings of process
*/
void TemplateHVControlCallback::initialize(const HVConfig& config) throw()
{
  try {
    load(config, false, true);
  } catch (const HVHandlerException& e) {
    LogFile::error(e.what());
  }
}

void TemplateHVControlCallback::update() throw(HVHandlerException)
{
  /*performed at the interval of hv.interval*/
}

void TemplateHVControlCallback::setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException)
{
  /*set channel switch (true:ON, false:OFF)*/
  LogFile::info("setswitch called : crate = %d, slot = %d, channel = %d, switch: %s",
                crate, slot, channel, (switchon ? "ON" : "OFF"));
}

void TemplateHVControlCallback::setRampUpSpeed(int crate, int slot, int channel, float voltage) throw(IOException)
{
  /*set voltage ramp up speed with unit of [V]*/
  LogFile::info("setrampup called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, voltage);
}

void TemplateHVControlCallback::setRampDownSpeed(int crate, int slot, int channel, float voltage) throw(IOException)
{
  /*set voltage ramp down speed with unit of [V]*/
  LogFile::info("setrampdown called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, voltage);
}

void TemplateHVControlCallback::setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException)
{
  /*set demand voltage with unit of [V]*/
  LogFile::info("setvoltagedemand called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, voltage);
}

void TemplateHVControlCallback::setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException)
{
  /*set voltage limit with unit of [V]*/
  LogFile::info("setvoltagelimit called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, voltage);
}

void TemplateHVControlCallback::setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException)
{
  /*set current limit with unit of [uA]*/
  LogFile::info("setcurrentlimit called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, current);
}

float TemplateHVControlCallback::getRampUpSpeed(int crate, int slot, int channel) throw(IOException)
{
  /*return voltage ramp up speed with unit of [V]*/
  return 1;
}

float TemplateHVControlCallback::getRampDownSpeed(int crate, int slot, int channel) throw(IOException)
{
  /*return voltage ramp down speed with unit of [V]*/
  return 1;
}

float TemplateHVControlCallback::getVoltageDemand(int crate, int slot, int channel) throw(IOException)
{
  /*return demand voltage with unit of [V]*/
  return 1;
}

float TemplateHVControlCallback::getVoltageLimit(int crate, int slot, int channel) throw(IOException)
{
  /*return voltage limit with unit of [V]*/
  return 1000;
}

float TemplateHVControlCallback::getCurrentLimit(int crate, int slot, int channel) throw(IOException)
{
  /*return current limit with unit of [uA]*/
  return 100;
}

float TemplateHVControlCallback::getVoltageMonitor(int crate, int slot, int channel) throw(IOException)
{
  /*return monitored voltage with unit of [V]*/
  return 800 + rand() % 20;
}

float TemplateHVControlCallback::getCurrentMonitor(int crate, int slot, int channel) throw(IOException)
{
  /*return monitored current with unit of [uA]*/
  return 10 + rand() % 10;
}

bool TemplateHVControlCallback::getSwitch(int crate, int slot, int channel) throw(IOException)
{
  /*return status of channel swicth (true:ON, false:OFF)*/
  return false;
}

int TemplateHVControlCallback::getState(int crate, int slot, int channel) throw(IOException)
{
  /*
    return channel status:
    OFF: power off,
    ON: power on without error
    OCP: trip due to over current
    OVP: trip due to over voltage
    ERR: another error
  */
  return HVMessage::OFF;
}

