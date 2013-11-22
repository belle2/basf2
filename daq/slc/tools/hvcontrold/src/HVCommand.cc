#include "HVCommand.h"
#include "HVState.h"

using namespace Belle2;

const HVCommand HVCommand::LOAD(101, "LOAD", "LOAD");
const HVCommand HVCommand::SWITCHON(102, "SWITCHON", "SWITCHON");
const HVCommand HVCommand::SWITCHOFF(103, "SWITCHOFF", "SWITCHOFF");

const HVCommand& HVCommand::operator=(const std::string& label) throw()
{
  if (label == LOAD._label) *this = LOAD;
  else if (label == SWITCHON._label) *this = SWITCHON;
  else if (label == SWITCHOFF._label) *this = SWITCHOFF;
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

