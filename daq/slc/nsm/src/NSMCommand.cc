#include "daq/slc/nsm/NSMCommand.h"

#include "daq/slc/nsm/NSMState.h"

using namespace Belle2;

const NSMCommand NSMCommand::OK(1, "OK");
const NSMCommand NSMCommand::ERROR(2, "ERROR");
const NSMCommand NSMCommand::FATAL(3, "FATAL");
const NSMCommand NSMCommand::STATE(4, "STATE");
const NSMCommand NSMCommand::LOG(5, "LOG");
const NSMCommand NSMCommand::VGET(11, "VGET");
const NSMCommand NSMCommand::VSET(12, "VSET");
const NSMCommand NSMCommand::VREPLY(13, "VREPLY");
const NSMCommand NSMCommand::VLISTGET(14, "VLISTGET");
const NSMCommand NSMCommand::VLISTSET(15, "VLISTSET");
const NSMCommand NSMCommand::NSMDATAGET(401, "NSMDATAGET");
const NSMCommand NSMCommand::NSMDATASET(402, "NSMDATASET");
const NSMCommand NSMCommand::DBSET(403, "DBSET");
const NSMCommand NSMCommand::DBGET(404, "DBGET");
const NSMCommand NSMCommand::DBLISTSET(405, "DBLISTSET");
const NSMCommand NSMCommand::DBLISTGET(406, "DBLISTGET");

const NSMCommand& NSMCommand::operator=(const std::string& label) throw()
{
  if (label == LOG.getLabel()) *this = LOG;
  else if (label == OK.getLabel()) *this = OK;
  else if (label == ERROR.getLabel()) *this = ERROR;
  else if (label == FATAL.getLabel()) *this = FATAL;
  else if (label == LOG.getLabel()) *this = LOG;
  else if (label == STATE.getLabel()) *this = STATE;
  else if (label == VGET.getLabel()) *this = VGET;
  else if (label == VSET.getLabel()) *this = VSET;
  else if (label == VREPLY.getLabel()) *this = VREPLY;
  else if (label == VLISTGET.getLabel()) *this = VLISTGET;
  else if (label == VLISTSET.getLabel()) *this = VLISTSET;
  else if (label == NSMDATAGET.getLabel()) *this = NSMDATAGET;
  else if (label == NSMDATASET.getLabel()) *this = NSMDATASET;
  else if (label == DBSET.getLabel()) *this = DBSET;
  else if (label == DBGET.getLabel()) *this = DBGET;
  else if (label == DBLISTSET.getLabel()) *this = DBLISTSET;
  else if (label == DBLISTGET.getLabel()) *this = DBLISTGET;
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
  else if (id == VSET.getId()) *this = VSET;
  else if (id == VGET.getId()) *this = VGET;
  else if (id == VREPLY.getId()) *this = VREPLY;
  else if (id == VLISTSET.getId()) *this = VLISTSET;
  else if (id == VLISTGET.getId()) *this = VLISTGET;
  else if (id == NSMDATASET.getId()) *this = NSMDATASET;
  else if (id == NSMDATAGET.getId()) *this = NSMDATAGET;
  else if (id == DBSET.getId()) *this = DBSET;
  else if (id == DBGET.getId()) *this = DBGET;
  else if (id == DBLISTSET.getId()) *this = DBLISTSET;
  else if (id == DBLISTGET.getId()) *this = DBLISTGET;
  else *this = Enum::UNKNOWN;
  return *this;
}

const NSMCommand& NSMCommand::operator=(const char* label) throw()
{
  if (label != NULL)  *this = std::string(label);
  else *this = Enum::UNKNOWN;
  return *this;
}

