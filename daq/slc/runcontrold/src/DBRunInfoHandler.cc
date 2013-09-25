#include "DBRunInfoHandler.hh"

#include <system/Date.hh>

#include <util/StringUtil.hh>

#include <iostream>
#include <sstream>

using namespace B2DAQ;

void DBRunInfoHandler::createRunConfigTable() throw(DBHandlerException)
{
  _db->execute("create table run_config (exp_no int, run_no int, run_type text, start_time timestamp, version int, operators text);");
}

void DBRunInfoHandler::createRunStatusTable() throw(DBHandlerException)
{
  _db->execute("create table run_status (exp_no int, run_no int, total_triggers int, start_time timestamp, end_time timestamp);");
}

void DBRunInfoHandler::writeRunConfigTable() throw(DBHandlerException)
{
  std::stringstream ss;
  ss << "insert into run_config values (" << _status->getExpNumber() << ", "
     << _status->getRunNumber() << ", '" << _config->getRunType() << "', '"
     << std::string(Date(_status->getStartTime()).toString()) << "',  "
     << _config->getVersion() << ", '" << _config->getOperators() << "');";
  _db->execute(ss.str());
}

void DBRunInfoHandler::writeRunStatusTable() throw(DBHandlerException)
{
  std::stringstream ss;
  ss << "insert into run_status values (" << _status->getExpNumber() << ", "
     << _status->getRunNumber() << ", " <<  _status->getTotalTriggers() << ", '"
     << std::string(Date(_status->getStartTime()).toString()) << "', '"
     << std::string(Date(_status->getEndTime()).toString()) << "');";
  _db->execute(ss.str());
}

