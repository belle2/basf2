#include "daq/slc/hvcontrol/cdc/CdcHVControlCallback.h"

#include "daq/slc/system/LogFile.h"

#include <cstdlib>

using namespace Belle2;

void CdcHVControlCallback::initialize(const HVConfig& hvconf) throw()
{

}

void CdcHVControlCallback::update() throw(HVHandlerException)
{

}

void CdcHVControlCallback::store(int index) throw(IOException)
{
  LogFile::notice("store called : index = %d", index);
}

void CdcHVControlCallback::recall(int index) throw(IOException)
{
  LogFile::notice("recall called : index = %d", index);
}

void CdcHVControlCallback::setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException)
{
  LogFile::info("setswitch called : crate = %d, slot = %d, channel = %d, switch: %s",
                crate, slot, channel, (switchon ? "ON" : "OFF"));
}

void CdcHVControlCallback::setRampUpSpeed(int crate, int slot, int channel, float voltage) throw(IOException)
{
  LogFile::info("setrampup called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, voltage);
}

void CdcHVControlCallback::setRampDownSpeed(int crate, int slot, int channel, float voltage) throw(IOException)
{
  LogFile::info("setrampdown called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, voltage);
}

void CdcHVControlCallback::setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException)
{
  LogFile::info("setvoltagedemand called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, voltage);
}

void CdcHVControlCallback::setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException)
{
  LogFile::info("setvoltagelimit called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, voltage);
}

void CdcHVControlCallback::setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException)
{
  LogFile::info("setcurrentlimit called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, current);
}

float CdcHVControlCallback::getRampUpSpeed(int crate, int slot, int channel) throw(IOException)
{
  LogFile::info("getrampupspeed called : crate = %d, slot = %d, channel = %d",
                crate, slot, channel);
  return getConfig().getChannel(crate, slot, channel).getRampUpSpeed();
}

float CdcHVControlCallback::getRampDownSpeed(int crate, int slot, int channel) throw(IOException)
{
  return getConfig().getChannel(crate, slot, channel).getRampDownSpeed();
}

float CdcHVControlCallback::getVoltageDemand(int crate, int slot, int channel) throw(IOException)
{
  return getConfig().getChannel(crate, slot, channel).getVoltageDemand();
}

float CdcHVControlCallback::getVoltageLimit(int crate, int slot, int channel) throw(IOException)
{
  return getConfig().getChannel(crate, slot, channel).getVoltageLimit();
}

float CdcHVControlCallback::getCurrentLimit(int crate, int slot, int channel) throw(IOException)
{
  return getConfig().getChannel(crate, slot, channel).getCurrentLimit();
}

float CdcHVControlCallback::getVoltageMonitor(int crate, int slot, int channel) throw(IOException)
{
  return 1990 + rand() % 20;
}

float CdcHVControlCallback::getCurrentMonitor(int crate, int slot, int channel) throw(IOException)
{
  return 9;
}

bool CdcHVControlCallback::getSwitch(int crate, int slot, int channel) throw(IOException)
{
  return true;
}

int CdcHVControlCallback::getState(int crate, int slot, int channel) throw(IOException)
{
  return HVMessage::ON;
}

