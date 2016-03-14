#include "daq/slc/hvcontrol/HVCommand.h"

#include "daq/slc/hvcontrol/HVState.h"

using namespace Belle2;

const HVCommand HVCommand::CONFIGURE(101, "HV_CONFIGURE");
const HVCommand HVCommand::TURNON(102, "HV_TURNON");
const HVCommand HVCommand::TURNOFF(103, "HV_TURNOFF");
const HVCommand HVCommand::STANDBY(111, "HV_STANDBY");
const HVCommand HVCommand::SHOULDER(112, "HV_SHOULDER");
const HVCommand HVCommand::PEAK(113, "HV_PEAK");
const HVCommand HVCommand::RECOVER(121, "HV_RECOVER");

const HVCommand& HVCommand::operator=(const std::string& label) throw()
{
  if (label == CONFIGURE.getLabel()) *this = CONFIGURE;
  else if (label == TURNON.getLabel()) *this = TURNON;
  else if (label == TURNOFF.getLabel()) *this = TURNOFF;
  else if (label == STANDBY.getLabel()) *this = STANDBY;
  else if (label == SHOULDER.getLabel()) *this = SHOULDER;
  else if (label == PEAK.getLabel()) *this = PEAK;
  else if (label == RECOVER.getLabel()) *this = RECOVER;
  else if (label == NSMCommand::OK.getLabel()) *this = NSMCommand::OK;
  else if (label == NSMCommand::ERROR.getLabel()) *this = NSMCommand::ERROR;
  else if (label == NSMCommand::VGET.getLabel()) *this = NSMCommand::VGET;
  else if (label == NSMCommand::VSET.getLabel()) *this = NSMCommand::VSET;
  else *this = Enum::UNKNOWN;
  return *this;
}

const HVCommand& HVCommand::operator=(const char* label) throw()
{
  if (label != NULL)  *this = std::string(label);
  else *this = Enum::UNKNOWN;
  return *this;
}

