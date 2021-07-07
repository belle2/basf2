/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/database/DAQLogDB.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/database/DBHandlerException.h>

#include <iostream>
#include <sstream>
#include <cstdio>
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
    std::string tablename_date = tablename + "_" + Date().toString("%Y");
    if (!db.checkTable("daqlog")) {
      db.execute("create table daqlog \n"
                 "(name text not null, \n"
                 "id bigserial, lastupdate timestamp, \n"
                 "UNIQUE(name));");
      db.execute("create index daqlog_id_index on daqlog(id);",
                 tablename.c_str(), tablename.c_str());
    }
    if (!db.checkTable(tablename_date)) {
      try {
        db.execute("insert into daqlog (name, lastupdate) values "
                   "('%s', current_timestamp);", tablename_date.c_str());
        db.execute("create table %s \n"
                   "(node  int not null, \n"
                   "priority int not null, \n"
                   "id bigserial, \n"
                   "date timestamp with time zone, \n"
                   "message text not null); ", tablename_date.c_str());
      } catch (const std::exception& e) {
        db.execute("update daqlog set lastupdate = current_timestamp where name = '%s';",
                   tablename_date.c_str());
      }
      db.execute("create index %s_id_index on %s(id);",
                 tablename_date.c_str(), tablename_date.c_str());
    }
    db.execute("select id,category from log_node where name = '" + log.getNodeName() + "';");
    DBRecordList record(db.loadRecords());
    int id = 1;
    //int cid = 1;
    if (record.size() > 0) {
      id = record[0].getInt("id");
      //cid = record[0].getInt("category");
    } else {
      db.execute("insert into log_node (name, category) values ('" + log.getNodeName() +
                 "', " + StringUtil::form("%d", log.getCategory()) + ") returning id,category;");
      DBRecordList record(db.loadRecords());
      id = record[0].getInt("id");
      //cid = record[0].getInt("category");
    }
    db.execute("insert into %s (node, priority, date, message) values "
               "(%d, %d, to_timestamp(%d), '%s');",
               tablename_date.c_str(), id, log.getPriority(),
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
  std::string tablename_date = tablename + "_" + Date().toString("%Y");
  DAQLogMessageList logs;
  try {
    if (!db.isConnected()) db.connect();
    if (nodename.size() > 0) {
      if (max > 0) {
        db.execute("select n.name, extract(epoch from l.date) date, l.priority, l.message"
                   " from %s as l, log_node as n, log_priority as p where l.node=n.id and p.id=l.priority and n.name = '%s' order by l.id desc limit %d;",
                   tablename_date.c_str(), nodename.c_str(), max);
      } else {
        db.execute("select n.name, extract(epoch from l.date) date, l.priority, l.message"
                   " from %s as l, log_node as n, log_priority as p where l.node=n.id and p.id=l.priority and n.name = '%s' order by l.id desc;",
                   tablename_date.c_str(), nodename.c_str());
      }
    } else {
      if (max > 0) {
        db.execute("select n.name, extract(epoch from l.date) date, l.priority, l.message"
                   " from %s as l, log_node as n, log_priority as p where l.node=n.id and p.id=l.priority order by l.id desc limit %d;",
                   tablename_date.c_str(), max);
      } else {
        db.execute("select n.name, extract(epoch from l.date) date, l.priority, l.message"
                   " from %s as l, log_node as n, log_priority as p where l.node=n.id and p.id=l.priority order by l.id desc;",
                   tablename_date.c_str());
      }
    }
    DBRecordList record_v(db.loadRecords());
    for (size_t i = 0; i < record_v.size(); i++) {
      DBRecord& record(record_v[i]);
      logs.push_back(DAQLogMessage(record.get("n.name"),
                                   (LogFile::Priority)(record.getInt("priority")),
                                   record.get("l.message"), Date(record.getInt("date"))));
    }
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
  }
  return logs;
}

DAQLogMessageList DAQLogDB::getLogs(DBInterface& db, const std::string& tablename,
                                    const std::string& nodename, const std::string& begin_date,
                                    const std::string& end_date, int max, int priority)
{
  std::string tablename_date = tablename + "_" + Date().toString("%Y");
  DAQLogMessageList logs;
  try {
    if (!db.isConnected()) db.connect();
    std::stringstream ss;
    ss << "select n.name, extract(epoch from l.date) date, l.priority, l.message"
       << " from " << tablename_date << " as l, log_node as n where l.node=n.id ";
    if (nodename.size() > 0) {
      ss << "and n.name = '" << nodename << "' ";
    }
    if (begin_date.size() > 0) {
      ss << "and l.date >= (timestamp '" << begin_date << "') ";
    }
    if (end_date.size() > 0) {
      ss << "and l.date <= (timestamp '" << end_date << "') ";
    }
    if (priority > 0) {
      ss << "and l.priority >= " << priority << " ";
    }
    ss << "order by l.id desc ";
    if (max > 0) ss << "limit " << max;
    db.execute(ss.str().c_str());
    DBRecordList record_v(db.loadRecords());
    for (size_t i = 0; i < record_v.size(); i++) {
      DBRecord& record(record_v[i]);
      logs.push_back(DAQLogMessage(record.get("name"),
                                   (LogFile::Priority)(record.getInt("priority")),
                                   record.get("message"), Date(record.getInt("date"))));
    }
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
  }
  return logs;
}
