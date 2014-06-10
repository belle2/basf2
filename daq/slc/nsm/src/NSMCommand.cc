#include "daq/slc/nsm/NSMCommand.h"

#include "daq/slc/nsm/NSMState.h"

using namespace Belle2;

const NSMCommand NSMCommand::OK(1, "OK");
const NSMCommand NSMCommand::ERROR(2, "ERROR");
const NSMCommand NSMCommand::FATAL(3, "FATAL");
const NSMCommand NSMCommand::STATE(4, "STATE");
const NSMCommand NSMCommand::LOG(5, "LOG");
const NSMCommand NSMCommand::NSMGET(401, "NSMGET");
const NSMCommand NSMCommand::NSMSET(402, "NSMSET");
const NSMCommand NSMCommand::DBSET(403, "DBSET");
const NSMCommand NSMCommand::DBGET(404, "DBGET");
const NSMCommand NSMCommand::LISTSET(405, "LISTSET");
const NSMCommand NSMCommand::LISTGET(406, "LISTGET");

const NSMCommand& NSMCommand::operator=(const std::string& label) throw()
{
  if (label == LOG.getLabel()) *this = LOG;
  else if (label == OK.getLabel()) *this = OK;
  else if (label == ERROR.getLabel()) *this = ERROR;
  else if (label == FATAL.getLabel()) *this = FATAL;
  else if (label == LOG.getLabel()) *this = LOG;
  else if (label == STATE.getLabel()) *this = STATE;
  else if (label == NSMGET.getLabel()) *this = NSMGET;
  else if (label == NSMSET.getLabel()) *this = NSMSET;
  else if (label == DBSET.getLabel()) *this = DBSET;
  else if (label == DBGET.getLabel()) *this = DBGET;
  else if (label == LISTSET.getLabel()) *this = LISTSET;
  else if (label == LISTGET.getLabel()) *this = LISTGET;
  else *this = Enum::UNKNOWN;
  return *this;
}

const NSMCommand& NSMCommand::operator=(int id) throw()
{
  if (id == LOG.getId()) *this = LOG;
  else if (id == OK.getId()) *this = OK;
  else if (id == ERROR.getId()) *this = ERROR;
  else if (id == FATAL.getId()) *this = FATAL;
  else if (id == LOG.getId()) *this = LOG;
  else if (id == STATE.getId()) *this = STATE;
  else if (id == NSMSET.getId()) *this = NSMSET;
  else if (id == NSMGET.getId()) *this = NSMGET;
  else if (id == DBSET.getId()) *this = DBSET;
  else if (id == DBGET.getId()) *this = DBGET;
  else if (id == LISTSET.getId()) *this = LISTSET;
  else if (id == LISTGET.getId()) *this = LISTGET;
  else *this = Enum::UNKNOWN;
  return *this;
}

const NSMCommand& NSMCommand::operator=(const char* label) throw()
{
  if (label != NULL)  *this = std::string(label);
  else *this = Enum::UNKNOWN;
  return *this;
}

