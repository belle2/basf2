/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/nsm/NSMCommand.h"

using namespace Belle2;

const NSMCommand NSMCommand::OK(1, "OK");
const NSMCommand NSMCommand::ERROR(2, "ERROR");
const NSMCommand NSMCommand::FATAL(3, "FATAL");
const NSMCommand NSMCommand::STATE(4, "STATE");
const NSMCommand NSMCommand::LOG(5, "LOG");
const NSMCommand NSMCommand::LOGSET(6, "LOGSET");
const NSMCommand NSMCommand::LOGGET(7, "LOGGET");
const NSMCommand NSMCommand::VGET(11, "VGET");
const NSMCommand NSMCommand::VSET(12, "VSET");
const NSMCommand NSMCommand::VREPLY(13, "VREPLY");
const NSMCommand NSMCommand::VLISTGET(14, "VLISTGET");
const NSMCommand NSMCommand::VLISTSET(15, "VLISTSET");
const NSMCommand NSMCommand::DATAGET(401, "DATAGET");
const NSMCommand NSMCommand::DATASET(402, "DATASET");
const NSMCommand NSMCommand::DBSET(403, "DBSET");
const NSMCommand NSMCommand::DBGET(404, "DBGET");
const NSMCommand NSMCommand::DBLISTSET(405, "DBLISTSET");
const NSMCommand NSMCommand::DBLISTGET(406, "DBLISTGET");
const NSMCommand NSMCommand::LOGLIST(407, "LOGLIST");

const NSMCommand& NSMCommand::operator=(const std::string& label)
{
  if (label == LOG.getLabel()) *this = LOG;
  else if (label == OK.getLabel()) *this = OK;
  else if (label == ERROR.getLabel()) *this = ERROR;
  else if (label == FATAL.getLabel()) *this = FATAL;
  else if (label == LOG.getLabel()) *this = LOG;
  else if (label == LOGSET.getLabel()) *this = LOGSET;
  else if (label == LOGGET.getLabel()) *this = LOGGET;
  else if (label == STATE.getLabel()) *this = STATE;
  else if (label == VGET.getLabel()) *this = VGET;
  else if (label == VSET.getLabel()) *this = VSET;
  else if (label == VREPLY.getLabel()) *this = VREPLY;
  else if (label == VLISTGET.getLabel()) *this = VLISTGET;
  else if (label == VLISTSET.getLabel()) *this = VLISTSET;
  else if (label == DATAGET.getLabel()) *this = DATAGET;
  else if (label == DATASET.getLabel()) *this = DATASET;
  else if (label == DBSET.getLabel()) *this = DBSET;
  else if (label == DBGET.getLabel()) *this = DBGET;
  else if (label == DBLISTSET.getLabel()) *this = DBLISTSET;
  else if (label == DBLISTGET.getLabel()) *this = DBLISTGET;
  else if (label == LOGLIST.getLabel()) *this = LOGLIST;
  else *this = Enum::UNKNOWN;
  return *this;
}

const NSMCommand& NSMCommand::operator=(int id)
{
  if (id == LOG.getId()) *this = LOG;
  else if (id == OK.getId()) *this = OK;
  else if (id == ERROR.getId()) *this = ERROR;
  else if (id == FATAL.getId()) *this = FATAL;
  else if (id == LOG.getId()) *this = LOG;
  else if (id == LOGSET.getId()) *this = LOGSET;
  else if (id == LOGGET.getId()) *this = LOGGET;
  else if (id == STATE.getId()) *this = STATE;
  else if (id == VSET.getId()) *this = VSET;
  else if (id == VGET.getId()) *this = VGET;
  else if (id == VREPLY.getId()) *this = VREPLY;
  else if (id == VLISTSET.getId()) *this = VLISTSET;
  else if (id == VLISTGET.getId()) *this = VLISTGET;
  else if (id == DATASET.getId()) *this = DATASET;
  else if (id == DATAGET.getId()) *this = DATAGET;
  else if (id == DBSET.getId()) *this = DBSET;
  else if (id == DBGET.getId()) *this = DBGET;
  else if (id == DBLISTSET.getId()) *this = DBLISTSET;
  else if (id == DBLISTGET.getId()) *this = DBLISTGET;
  else if (id == LOGLIST.getId()) *this = LOGLIST;
  else *this = Enum::UNKNOWN;
  return *this;
}

const NSMCommand& NSMCommand::operator=(const char* label)
{
  if (label != NULL)  *this = std::string(label);
  else *this = Enum::UNKNOWN;
  return *this;
}

