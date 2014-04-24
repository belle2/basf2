#include "daq/rfarm/manager/RFCommand.h"

using namespace Belle2;

const RFCommand RFCommand::RF_CONFIGURE(601, "RF_CONFIGURE");
const RFCommand RFCommand::RF_UNCONFIGURE(602, "RF_UNCONFIGURE");
const RFCommand RFCommand::RF_START(603, "RF_START");
const RFCommand RFCommand::RF_STOP(604, "RF_STOP");
const RFCommand RFCommand::RF_RESTART(605, "RF_RESTART");
const RFCommand RFCommand::RF_PAUSE(606, "RF_PAUSE");
const RFCommand RFCommand::RF_RESUME(607, "RF_RESUME");
const RFCommand RFCommand::RF_STATUS(608, "RF_STATUS");

const RFCommand& RFCommand::operator=(const std::string& label) throw()
{
  if (label == RF_CONFIGURE._label) *this = RF_CONFIGURE;
  else if (label == RF_UNCONFIGURE._label) *this = RF_UNCONFIGURE;
  else if (label == RF_START._label) *this = RF_START;
  else if (label == RF_STOP._label) *this = RF_STOP;
  else if (label == RF_RESTART._label) *this = RF_RESTART;
  else if (label == RF_PAUSE._label) *this = RF_PAUSE;
  else if (label == RF_RESUME._label) *this = RF_RESUME;
  else if (label == RF_STATUS._label) *this = RF_STATUS;
  else if (label == NSMCommand::OK.getLabel()) *this = NSMCommand::OK;
  else if (label == NSMCommand::ERROR.getLabel()) *this = NSMCommand::ERROR;
  else *this = Enum::UNKNOWN;
  return *this;
}

const RFCommand& RFCommand::operator=(const char* label) throw()
{
  if (label != NULL)  *this = std::string(label);
  else *this = Enum::UNKNOWN;
  return *this;
}

