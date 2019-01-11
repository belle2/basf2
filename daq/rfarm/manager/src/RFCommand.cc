#include "daq/rfarm/manager/RFCommand.h"

using namespace Belle2;

const RFCommand RFCommand::CONFIGURE(601, "RF_CONFIGURE");
const RFCommand RFCommand::UNCONFIGURE(602, "RF_UNCONFIGURE");
const RFCommand RFCommand::START(603, "RF_START");
const RFCommand RFCommand::STOP(604, "RF_STOP");
const RFCommand RFCommand::RESTART(605, "RF_RESTART");
const RFCommand RFCommand::PAUSE(606, "RF_PAUSE");
const RFCommand RFCommand::RESUME(607, "RF_RESUME");
const RFCommand RFCommand::STATUS(608, "RF_STATUS");

const RFCommand& RFCommand::operator=(const std::string& label)
{
  if (label == CONFIGURE.getLabel()) *this = CONFIGURE;
  else if (label == UNCONFIGURE.getLabel()) *this = UNCONFIGURE;
  else if (label == START.getLabel()) *this = START;
  else if (label == STOP.getLabel()) *this = STOP;
  else if (label == RESTART.getLabel()) *this = RESTART;
  else if (label == PAUSE.getLabel()) *this = PAUSE;
  else if (label == RESUME.getLabel()) *this = RESUME;
  else if (label == STATUS.getLabel()) *this = STATUS;
  else if (label == NSMCommand::OK.getLabel()) *this = NSMCommand::OK;
  else if (label == NSMCommand::ERROR.getLabel()) *this = NSMCommand::ERROR;
  else *this = Enum::UNKNOWN;
  return *this;
}

const RFCommand& RFCommand::operator=(const char* label)
{
  if (label != NULL)  *this = std::string(label);
  else *this = Enum::UNKNOWN;
  return *this;
}

