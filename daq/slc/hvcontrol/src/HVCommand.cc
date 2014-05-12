#include "daq/slc/hvcontrol/HVCommand.h"

#include "daq/slc/hvcontrol/HVState.h"

using namespace Belle2;

const HVCommand HVCommand::CONFIGURE(101, "CONFIGURE");
const HVCommand HVCommand::TURNON(102, "TURNON");
const HVCommand HVCommand::TURNOFF(103, "TURNOFF");
const HVCommand HVCommand::STANDBY(111, "STANDBY");
const HVCommand HVCommand::SHOULDER(112, "SHOULDER");
const HVCommand HVCommand::PEAK(113, "PEAK");
const HVCommand HVCommand::RECOVER(121, "RECOVER");

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
  else if (label == NSMCommand::NSMGET.getLabel()) *this = NSMCommand::NSMGET;
  else if (label == NSMCommand::NSMSET.getLabel()) *this = NSMCommand::NSMSET;
  else if (label == NSMCommand::DBGET.getLabel()) *this = NSMCommand::DBGET;
  else if (label == NSMCommand::DBSET.getLabel()) *this = NSMCommand::DBSET;
  else *this = Enum::UNKNOWN;
  return *this;
}

const HVCommand& HVCommand::operator=(const char* label) throw()
{
  if (label != NULL)  *this = std::string(label);
  else *this = Enum::UNKNOWN;
  return *this;
}

