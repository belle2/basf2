#include "daq/slc/dqm/DQMCommand.h"

using namespace Belle2;

const DQMCommand DQMCommand::DQMDIRGET(701, "DQMDIRGET");
const DQMCommand DQMCommand::DQMDIRSET(702, "DQMDIRSET");
const DQMCommand DQMCommand::DQMHISTGET(703, "DQMHISTGET");
const DQMCommand DQMCommand::DQMHISTSET(704, "DQMHISTSET");
const DQMCommand DQMCommand::DQMLISTGET(705, "DQMLISTGET ");
const DQMCommand DQMCommand::DQMLISTSET(706, "DQMLISTSET ");
const DQMCommand DQMCommand::DQMFILEGET(707, "DQMFILEGET");
const DQMCommand DQMCommand::DQMFILESET(708, "DQMFILESET");
const DQMCommand DQMCommand::DQMRESET(709, "DQMRESET");

const DQMCommand& DQMCommand::operator=(const std::string& label) throw()
{
  if (NSMCommand::operator=(label) != Enum::UNKNOWN) return *this;
  else if (label == DQMDIRGET.getLabel()) *this = DQMDIRGET;
  else if (label == DQMDIRSET.getLabel()) *this = DQMDIRSET;
  else if (label == DQMHISTGET.getLabel()) *this = DQMHISTGET;
  else if (label == DQMHISTSET.getLabel()) *this = DQMHISTSET;
  else if (label == DQMLISTGET.getLabel()) *this = DQMLISTGET;
  else if (label == DQMLISTSET.getLabel()) *this = DQMLISTSET;
  else if (label == DQMFILEGET.getLabel()) *this = DQMFILEGET;
  else if (label == DQMFILESET.getLabel()) *this = DQMFILESET;
  else if (label == DQMRESET.getLabel()) *this = DQMRESET;
  else *this = Enum::UNKNOWN;
  return *this;
}

const DQMCommand& DQMCommand::operator=(int id) throw()
{
  if (NSMCommand::operator=(id) != Enum::UNKNOWN) return *this;
  else if (id == DQMDIRGET.getId()) *this = DQMDIRGET;
  else if (id == DQMDIRSET.getId()) *this = DQMDIRSET;
  else if (id == DQMHISTGET.getId()) *this = DQMHISTGET;
  else if (id == DQMHISTSET.getId()) *this = DQMHISTSET;
  else if (id == DQMLISTGET.getId()) *this = DQMLISTGET;
  else if (id == DQMLISTSET.getId()) *this = DQMLISTSET;
  else if (id == DQMFILEGET.getId()) *this = DQMFILEGET;
  else if (id == DQMFILESET.getId()) *this = DQMFILESET;
  else if (id == DQMRESET.getId()) *this = DQMRESET;
  else *this = Enum::UNKNOWN;
  return *this;
}

const DQMCommand& DQMCommand::operator=(const char* label) throw()
{
  if (label != NULL)  *this = std::string(label);
  else *this = Enum::UNKNOWN;
  return *this;
}

