#include "daq/slc/database/DAQLogDB.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

using namespace Belle2;

bool DAQLogDB::createLog(DBInterface& db, const std::string& tablename,
                         const DAQLogMessage& log)
{
  std::stringstream ss;
  try {
    if (!db.isConnected()) db.connect();
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
    return false;
  }
  try {
    if (!db.checkTable(tablename)) {
      db.execute("create table %s \n"
                 "(node  varchar(16) not null, \n"
                 "priority varchar(16) not null, \n"
                 "id bigserial, \n"
                 "date timestamp with time zone, \n"
                 "message text not null); ", tablename.c_str());
      db.execute("create index %s_id_index on %s(id);",
                 tablename.c_str(), tablename.c_str());
    }
    db.execute("insert into %s (node, priority, date, message) values "
               "('%s', '%s', to_timestamp(%d), '%s');",
               tablename.c_str(), log.getNodeName().c_str(), log.getPriorityText().c_str(),
               log.getDateInt(), log.getMessage().c_str());
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
    return false;
  }
  return true;
}

DAQLogMessageList DAQLogDB::getLogs(DBInterface& db, const std::string& tablename,
                                    const std::string& nodename, int max)
{
  DAQLogMessageList logs;
  try {
    if (!db.isConnected()) db.connect();
    if (nodename.size() > 0) {
      if (max > 0) {
        db.execute("select node, extract(epoch from date) date, priority, message"
                   " from %s where node = '%s' order by id desc limit %d;",
                   tablename.c_str(), nodename.c_str(), max);
      } else {
        db.execute("select node, extract(epoch from date) date, priority, message"
                   " from %s where node = '%s' order by id desc;",
                   tablename.c_str(), nodename.c_str());
      }
    } else {
      if (max > 0) {
        db.execute("select node, extract(epoch from date) date, priority, message"
                   " from %s order by id desc limit %d;", tablename.c_str(), max);
      } else {
        db.execute("select node, extract(epoch from date) date, priority, message"
                   " from %s order by id desc;", tablename.c_str());
      }
    }
    DBRecordList record_v(db.loadRecords());
    for (size_t i = 0; i < record_v.size(); i++) {
      DBRecord& record(record_v[i]);
      logs.push_back(DAQLogMessage(record.get("node"),
                                   LogFile::getPriority(record.get("priority")),
                                   record.get("message"), Date(record.getInt("date"))));
    }
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
  }
  return logs;
}

DAQLogMessageList DAQLogDB::getLogs(DBInterface& db, const std::string& tablename,
                                    const std::string& nodename, const std::string& begin_date,
                                    const std::string& end_date, int max)
{
  DAQLogMessageList logs;
  try {
    if (!db.isConnected()) db.connect();
    std::stringstream ss;
    ss << "select node, extract(epoch from date) date, priority, message "
       << "from " << tablename << " ";
    bool hasand = false;
    if (nodename.size() > 0) {
      hasand = true;
      ss << "where node = '" << nodename << "' ";
    }
    if (begin_date.size() > 0) {
      if (hasand) ss << "and ";
      else {
        ss << "where ";
        hasand = true;
      }
      ss << "date >= '" << begin_date << "' ";
    }
    if (end_date.size() > 0) {
      if (hasand) ss << "and ";
      else {
        ss << "where ";
      }
      ss << "date <= '" << end_date << "' ";
    }
    ss << "order by id desc ";
    if (max > 0) ss << "limit " << max;
    db.execute(ss.str().c_str());
    DBRecordList record_v(db.loadRecords());
    for (size_t i = 0; i < record_v.size(); i++) {
      DBRecord& record(record_v[i]);
      logs.push_back(DAQLogMessage(record.get("node"),
                                   LogFile::getPriority(record.get("priority")),
                                   record.get("message"), Date(record.getInt("date"))));
    }
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
  }
  return logs;
}
