#include "daq/slc/hvcontrol/HVCommand.h"

#include "daq/slc/hvcontrol/HVState.h"

using namespace Belle2;

const HVCommand HVCommand::CONFIGURE(101, "CONFIGURE");
const HVCommand HVCommand::TURNON(102, "TURNON");
const HVCommand HVCommand::TURNOFF(103, "TURNOFF");
const HVCommand HVCommand::RAMPUP(104, "RAMPUP");
const HVCommand HVCommand::RAMPDOWN(105, "RAMPDOWN");
const HVCommand HVCommand::STANDBY(111, "STANDBY");
const HVCommand HVCommand::STANDBY2(112, "STANDBY2");
const HVCommand HVCommand::STANDBY3(113, "STANDBY3");
const HVCommand HVCommand::PEAK(114, "PEAK");
const HVCommand HVCommand::RECOVER(121, "RECOVER");

const HVCommand& HVCommand::operator=(const std::string& label) throw()
{
  if (label == CONFIGURE._label) *this = CONFIGURE;
  else if (label == TURNON._label) *this = TURNON;
  else if (label == TURNOFF._label) *this = TURNOFF;
  else if (label == RAMPUP._label) *this = RAMPUP;
  else if (label == RAMPDOWN._label) *this = RAMPDOWN;
  else if (label == STANDBY._label) *this = STANDBY;
  else if (label == STANDBY2._label) *this = STANDBY2;
  else if (label == STANDBY3._label) *this = STANDBY3;
  else if (label == PEAK._label) *this = PEAK;
  else if (label == RECOVER._label) *this = RECOVER;
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

