#include "daq/slc/hvcontrol/arichlv/ArichlvControlCallback.h"
#include "daq/slc/system/LogFile.h"
#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/DBObjectLoader.h>
#include <daq/slc/system/PThread.h>

#include <cstdlib>
#include "daq/slc/hvcontrol/arichlv/MPOD.h"
#include <math.h>
using namespace Belle2;
const int MaxCrates = 10;
static HSNMP crateHsnmp[MaxCrates];

/*
translation of the slot and channel in a HW channel number
*/
int ArichlvControlCallback::MPODCH(int slot, int channel)
{

  if (slot   >= MaxSlotsPerCrate  || slot < 0) return -1;
  if (channel >= MaxChannelsPerSlot || channel < 0) return -1;
  return ((slot) * 100 + (channel));
}


/*
Test of the range and initialization
*/
int ArichlvControlCallback::checkRange(int crate, int slot, int channel, int linenum)
{

  if (!crateHsnmp[crate] || MPODCH(slot, channel) < 0) {
    LogFile::error("Wrong parameters requested  crate = %d, slot = %d, channel = %d at line %d in %s", crate, slot, channel, linenum,
                   __FILE__);
    return 0;
  } else {
    return 1;
  }
}

/*
  Setting parameters from database at beginings of process
*/
void ArichlvControlCallback::initialize(const HVConfig& config) throw()
{

  MPOD_Start();
  for (int i = 0; i < MaxCrates; i++) crateHsnmp[i] = 0;
  const HVCrateList& crate_v(config.getCrates());
  for (HVCrateList::const_iterator icrate = crate_v.begin();
       icrate != crate_v.end(); icrate++) {
    const HVCrate& crate(*icrate);
    int crateid = crate.getId();
    std::string host = crate.getName();
    if (crateid < 0 || crateid >= MaxCrates) {
      LogFile::error("Crate %d Wiener MPOD at " + host + " cannot be initialized. Wrong crate number", crateid);
      continue;
    }
    LogFile::info("Crate %d Wiener MPOD at " + host + " will now be initialized", crateid);

    if (crateHsnmp[crateid] == 0)  crateHsnmp[crateid] = MPOD_Open(host.c_str());
    if (crateHsnmp[crateid] == 0) LogFile::error("Initialize error %d", __LINE__);
    LogFile::info("Wiener MPOD at " + host + " Initialized");
    for (int module = 0; module < MaxSlotsPerCrate; module++)
      LogFile::info("ModuleDescription crate %d module %d =>%s", crateid, module, getModuleDescription(crateHsnmp[crateid], module));
  }

  try {
    //  load(config, false, true);
  } catch (const HVHandlerException& e) {
    LogFile::error(e.what());
  }

}



void ArichlvControlCallback::update() throw(HVHandlerException)
{
  /*
  const HVConfig& config = getConfig();
  const HVCrateList& crate_v(config.getCrates());
  for (HVCrateList::const_iterator icrate = crate_v.begin();
       icrate != crate_v.end(); icrate++) {
    const HVCrate& crate(*icrate);
    const HVChannelList& channel_v(crate.getChannels());
    int crateid = crate.getId();
    for (HVChannelList::const_iterator ichannel = channel_v.begin();
         ichannel != channel_v.end(); ichannel++) {
      const HVChannel& channel(*ichannel);
      unsigned int slot = channel.getSlot();
      unsigned int ch   = channel.getChannel();
      if (!checkRange(crateid, slot, ch, __LINE__)) continue;
      float vset  = getOutputVoltage(crateHsnmp[crateid], MPODCH(slot, ch));
      int   onoff = getChannelSwitch(crateHsnmp[crateid], MPODCH(slot, ch));
      float vmon  = getOutputSenseMeasurement(crateHsnmp[crateid], MPODCH(slot, ch));
      float imon  = getCurrentMeasurement(crateHsnmp[crateid], MPODCH(slot, ch));
    }
  }
  LogFile::debug("ArichlvControlCallback::update()");
  */
  /*performed at the interval of hv.interval*/
}

void ArichlvControlCallback::setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException)
{
  m_mutex.lock();
  int setval = (switchon == true) ? 1 : 0;
  if (checkRange(crate, slot, channel, __LINE__))
    setChannelSwitch(crateHsnmp[crate], MPODCH(slot, channel), setval);
  /*set channel switch (true:ON, false:OFF)*/
  LogFile::info("setswitch called : crate = %d, slot = %d, channel = %d, switch: %d %s",
                crate, slot, channel, setval, (switchon ? "ON" : "OFF"));
  m_mutex.unlock();
}

void ArichlvControlCallback::setRampUpSpeed(int crate, int slot, int channel, float voltage) throw(IOException)
{
  m_mutex.lock();
  /*set voltage ramp up speed with unit of [V]*/
  if (checkRange(crate, slot, channel, __LINE__))
    setOutputRiseRate(crateHsnmp[crate], MPODCH(slot, channel), voltage);

  LogFile::debug("setrampup called : crate = %d, slot = %d, channel = %d, voltage: %f ---> Module function common for slot",
                 crate, slot, channel, voltage);
  m_mutex.unlock();
}

void ArichlvControlCallback::setRampDownSpeed(int crate, int slot, int channel, float voltage) throw(IOException)
{
  m_mutex.lock();
  /*set voltage ramp down speed with unit of [V]*/
  LogFile::debug("setrampdown called : not implemeted in MPOD");
  m_mutex.unlock();
}

void ArichlvControlCallback::setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException)
{
  m_mutex.lock();
  /*set demand voltage with unit of [V]*/
  if (checkRange(crate, slot, channel, __LINE__))
    setOutputVoltage(crateHsnmp[crate], MPODCH(slot, channel), fabs(voltage));
  LogFile::debug("setvoltagedemand called : crate = %d, slot = %d, channel = %d, voltage: %f",
                 crate, slot, channel, voltage);
  m_mutex.unlock();
}

void ArichlvControlCallback::setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException)
{
  m_mutex.lock();
  /*set voltage limit with unit of [V]*/
  LogFile::warning("setvoltagelimit called : not implemeted in MPOD");
  m_mutex.unlock();
}

void ArichlvControlCallback::setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException)
{
  m_mutex.lock();
  /*set current limit with unit of [uA]*/
  if (checkRange(crate, slot, channel, __LINE__))
    setOutputCurrent(crateHsnmp[crate], MPODCH(slot, channel), current);
  LogFile::debug("setcurrentlimit called : crate = %d, slot = %d, channel = %d, voltage: %f",
                 crate, slot, channel, current);
  m_mutex.unlock();
}

float ArichlvControlCallback::getRampUpSpeed(int crate, int slot, int channel) throw(IOException)
{
  if (!checkRange(crate, slot, 0, __LINE__)) return 0;
  m_mutex.lock();
  float ret = getModuleRampSpeedVoltage(crateHsnmp[crate], slot);
  //float ret = getModuleRampSpeedVoltage(crateHsnmp[crate], MPODCH(slot, channel));//yone
  /*return voltage ramp up speed with unit of [V]*/
  m_mutex.unlock();
  return ret;
}

float ArichlvControlCallback::getRampDownSpeed(int crate, int slot, int channel) throw(IOException)
{
  if (!checkRange(crate, slot, 0, __LINE__)) return 0;
  m_mutex.lock();
  float ret = getModuleRampSpeedVoltage(crateHsnmp[crate], slot);
  /*return voltage ramp down speed with unit of [V]*/
  m_mutex.unlock();
  return ret;
}

float ArichlvControlCallback::getVoltageDemand(int crate, int slot, int channel) throw(IOException)
{
  if (!checkRange(crate, slot, channel, __LINE__)) return 0;
  m_mutex.lock();
  float ret = getOutputVoltage(crateHsnmp[crate], MPODCH(slot, channel));
  /*return demand voltage with unit of [V]*/
  m_mutex.unlock();
  return ret;
}

float ArichlvControlCallback::getVoltageLimit(int crate, int slot, int channel) throw(IOException)
{
  if (!checkRange(crate, slot, 0, __LINE__)) return 0;
  m_mutex.lock();
  float ret = getModuleHardwareLimitVoltage(crateHsnmp[crate], slot);
  /*return voltage limit with unit of [V]*/
  m_mutex.unlock();
  return ret;
}

float ArichlvControlCallback::getCurrentLimit(int crate, int slot, int channel) throw(IOException)
{
  if (!checkRange(crate, slot, channel, __LINE__)) return 0;
  m_mutex.lock();
  //  float ret = getModuleHardwareLimitCurrent(crateHsnmp[crate], slot);
  float ret = getOutputCurrent(crateHsnmp[crate], MPODCH(slot, channel));
  /*return current limit with unit of [uA]*/
  m_mutex.unlock();
  return ret;
}

float ArichlvControlCallback::getVoltageMonitor(int crate, int slot, int channel) throw(IOException)
{
  if (!checkRange(crate, slot, channel, __LINE__)) return 0;
  m_mutex.lock();
  double  ret = getOutputSenseMeasurement(crateHsnmp[crate], MPODCH(slot, channel));
  //  double  ret = getOutputTerminalMeasurement(crateHsnmp[crate], MPODCH(slot, channel));//yone
  /*return monitored voltage with unit of [V]*/
  m_mutex.unlock();
  return ret;
}

float ArichlvControlCallback::getCurrentMonitor(int crate, int slot, int channel) throw(IOException)
{
  if (!checkRange(crate, slot, channel, __LINE__)) return 0;
  m_mutex.lock();
  double ret = getCurrentMeasurement(crateHsnmp[crate], MPODCH(slot, channel));
  /*return monitored current with unit of [uA]*/
  m_mutex.unlock();
  return ret;
}

bool ArichlvControlCallback::getSwitch(int crate, int slot, int channel) throw(IOException)
{
  if (!checkRange(crate, slot, channel, __LINE__)) return 0;
  m_mutex.lock();
  int status = getChannelSwitch(crateHsnmp[crate], MPODCH(slot, channel));
  /*return status of channel swicth (true:ON, false:OFF)*/
  m_mutex.unlock();
  return (status) ? true : false;
}

int ArichlvControlCallback::getState(int crate, int slot, int channel) throw(IOException)
{

  enum MpodStatus {
    outputOn                        = 0x80,
    outputInhibit                   = 0x40,
    outputFailureMinSenseVoltage    = 0x20,
    outputFailureMaxSenseVoltage    = 0x10,
    outputFailureMaxTerminalVoltage = 0x8,
    outputFailureMaxCurrent         = 0x4,
    outputFailureMaxTemperature     = 0x2,
    outputFailureMaxPower           = 0x1,
    outpitReserved                  = 0x8000,
    outputFailureTimeout            = 0x4000,
    outputCurrentLimited            = 0x2000,
    outputRampUp                    = 0x1000,
    outputRampDown                  = 0x800,
    outputEnableKill                = 0x400,
    outputEmergencyOff              = 0x200,
    outputAdjusting                 = 0x100,
    outputConstantVoltage           = 0x80000,
    outputLowCurrentRange           = 0x40000,
    outputCurrentBoundsExceeded     = 0x20000,
    outputFailureCurrentLimit       = 0x10000
  };

  if (!checkRange(crate, slot, channel, __LINE__)) return 0;
  m_mutex.lock();
  int status = getOutputStatus(crateHsnmp[crate], MPODCH(slot, channel));
  static char strstatus[0xFFF] = "";
  strstatus[0] = 0;
  if (status &  outputOn) {sprintf(strstatus, "%s On", strstatus); }
  if (status &  outputInhibit) {sprintf(strstatus, "%s Inhibit", strstatus); }
  if (status &  outputFailureMinSenseVoltage) {sprintf(strstatus, "%s FailureMinSenseVoltage", strstatus); }
  if (status &  outputFailureMaxSenseVoltage) {sprintf(strstatus, "%s FailureMaxSenseVoltage", strstatus); }
  if (status &  outputFailureMaxTerminalVoltage) {sprintf(strstatus, "%s FailureMaxTerminalVoltage", strstatus); }
  if (status &  outputFailureMaxCurrent) {sprintf(strstatus, "%s FailureMaxCurrent", strstatus); }
  if (status &  outputFailureMaxTemperature) {sprintf(strstatus, "%s FailureMaxTemperature", strstatus); }
  if (status &  outputFailureMaxPower) {sprintf(strstatus, "%s FailureMaxPower", strstatus); }
  if (status &  outputFailureTimeout) {sprintf(strstatus, "%s FailureTimeout", strstatus); }
  if (status &  outputCurrentLimited) {sprintf(strstatus, "%s CurrentLimited", strstatus); }
  if (status &  outputRampUp) {sprintf(strstatus, "%s RampUp", strstatus); }
  if (status &  outputRampDown) {sprintf(strstatus, "%s RampDown", strstatus); }
  if (status &  outputEnableKill) {sprintf(strstatus, "%s EnableKill", strstatus); }
  if (status &  outputEmergencyOff) {sprintf(strstatus, "%s EmergencyOff", strstatus); }
  if (status &  outputAdjusting) {sprintf(strstatus, "%s Adjusting", strstatus); }
  if (status &  outputConstantVoltage) {sprintf(strstatus, "%s ConstantVoltage", strstatus); }
  if (status &  outputLowCurrentRange) {sprintf(strstatus, "%s LowCurrentRange", strstatus); }
  if (status &  outputCurrentBoundsExceeded) {sprintf(strstatus, "%s CurrentBoundsExceeded", strstatus); }
  if (status &  outputFailureCurrentLimit) {sprintf(strstatus, "%s FailureCurrentLimit", strstatus); }
  if (!status) {sprintf(strstatus, "%s Off", strstatus); }

  m_mutex.unlock();
  LogFile::debug("getState : c= %d, s= %d, ch= %d : status=%05x %s", crate, slot, channel, status, strstatus);

  if (status &  outputInhibit) return HVMessage::OFF;
  if (status &  outputFailureMinSenseVoltage) return HVMessage::ERR;
  if (status &  outputFailureMaxSenseVoltage) return HVMessage::ERR;
  if (status &  outputFailureMaxTerminalVoltage) return HVMessage::ERR;
  if (status &  outputFailureMaxCurrent) return HVMessage::ERR;
  if (status &  outputFailureMaxTemperature) return HVMessage::ERR;
  if (status &  outputFailureMaxPower) return HVMessage::ERR;
  if (status &  outputFailureTimeout) return HVMessage::ERR;
  if (status &  outputCurrentLimited) return HVMessage::OCP;
  if (status &  outputRampUp) return HVMessage::RAMPUP;
  if (status &  outputRampDown) return HVMessage::RAMPDOWN;
  if (status &  outputEnableKill) return HVMessage::ERR;
  if (status &  outputEmergencyOff) return HVMessage::ETRIP;
  if (status &  outputAdjusting) return HVMessage::ON;
  if (status &  outputConstantVoltage) return HVMessage::OVP;
  if (status &  outputLowCurrentRange) return HVMessage::ERR;
  if (status &  outputCurrentBoundsExceeded) return HVMessage::ERR;
  if (status &  outputFailureCurrentLimit) return HVMessage::ERR;
  if (status &  outputOn) return HVMessage::ON;
  /*

      OFF = 0, // power off,
      ON,      // power on without error
      OCP,     // trip due to over current
      OVP,     // trip due to over voltage
      ERR,     // another error
      RAMPUP,  // ramping up voltage
      RAMPDOWN, // ramping down voltage
      TRIP,     // Trip due to current trip
      ETRIP,    // External trip
      INTERLOCK // Inter lock
  */

  return HVMessage::OFF;
}


