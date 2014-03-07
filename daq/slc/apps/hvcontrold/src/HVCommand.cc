#include "daq/slc/apps/hvcontrold/HVCommand.h"

#include "daq/slc/apps/hvcontrold/HVState.h"

using namespace Belle2;

const HVCommand HVCommand::CONFIGURE(101, "CONFIGURE", "CONFIGURE");
const HVCommand HVCommand::SAVE(102, "SAVE", "SAVE");
const HVCommand HVCommand::TURNON(103, "TURNON", "TURNON");
const HVCommand HVCommand::TURNOFF(104, "TURNOFF", "TURNOFF");
const HVCommand HVCommand::RAMPUP(105, "RAMPUP", "RAMPUP");
const HVCommand HVCommand::RAMPDOWN(106, "RAMPDOWN", "RAMPDOWN");
const HVCommand HVCommand::STANDBY(111, "STANDBY", "STANDBY");
const HVCommand HVCommand::STANDBY2(112, "STANDBY2", "STANDBY2");
const HVCommand HVCommand::STANDBY3(113, "STANDBY3", "STANDBY3");
const HVCommand HVCommand::PEAK(114, "PEAK", "PEAK");
const HVCommand HVCommand::RECOVER(121, "RECOVER", "RECOVER");

const HVCommand& HVCommand::operator=(const std::string& label) throw()
{
  if (label == CONFIGURE._label) *this = CONFIGURE;
  else if (label == SAVE._label) *this = SAVE;
  else if (label == TURNON._label) *this = TURNON;
  else if (label == TURNOFF._label) *this = TURNOFF;
  else if (label == RAMPUP._label) *this = RAMPUP;
  else if (label == RAMPDOWN._label) *this = RAMPDOWN;
  else if (label == STANDBY._label) *this = STANDBY;
  else if (label == STANDBY2._label) *this = STANDBY2;
  else if (label == STANDBY3._label) *this = STANDBY3;
  else if (label == PEAK._label) *this = PEAK;
  else if (label == RECOVER._label) *this = RECOVER;
  else if (label == Command::OK.getLabel()) *this = Command::OK;
  else if (label == Command::ERROR.getLabel()) *this = Command::ERROR;
  else *this = Enum::UNKNOWN;
  return *this;
}

const HVCommand& HVCommand::operator=(const char* label) throw()
{
  if (label != NULL)  *this = std::string(label);
  else *this = Enum::UNKNOWN;
  return *this;
}

