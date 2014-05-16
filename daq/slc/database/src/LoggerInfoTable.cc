#include "daq/slc/database/LoggerInfoTable.h"

#include <daq/slc/base/StringUtil.h>

#include <daq/slc/system/LogFile.h>

#include <sstream>
#include <iostream>

using namespace Belle2;

const std::string LoggerInfo::getSQL() const throw()
{
  return StringUtil::form("select loggerid from loggerid('%s', '%s', %d)",
                          getNode().c_str(), getTable().c_str(),
                          getRevision());
  /*
  return StringUtil::form("select l.id from loggerinfo as l, nodeinfo as n, tableinfo as t "
        "where l.nodeid = n.id and l.tableid = t.id "
        "and t.name = '%s' and t.revision = %d "
        "and n.name = '%s' order by l.id desc limit 1",
                          getTable().c_str(), getRevision(), getNode().c_str());
  */
}

LoggerInfoList LoggerInfoTable::getList(const std::string& nodename)
{
  LoggerInfoList info_v;
  if (m_db != NULL) {
    try {
      m_db->execute("select * from loggernames('%s');", nodename.c_str());
      DBRecordList record_v(m_db->loadRecords());
      for (DBRecordList::iterator it = record_v.begin();
           it != record_v.end(); it++) {
        DBRecord& record(*it);
        int id = record.getInt("id");
        if (id > 0) {
          info_v.push_back(LoggerInfo(record.get("node"),
                                      record.get("table"), id));
        }
      }
    } catch (const DBHandlerException& e) {
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
  return info_v;
}

LoggerInfo LoggerInfoTable::get(int configid)
{
  LoggerInfo info;
  if (configid > 0 && m_db != NULL) {
    try {
      std::stringstream ss;
      ss << "select id, "
         "(select name from nodeinfo where id = nodeid) as node, "
         "(select name from tableinfo where id = tableid) as table "
         "from lofferinfo where id = " << configid << ";";
      m_db->execute(ss.str().c_str());
      DBRecordList record_v(m_db->loadRecords());
      for (DBRecordList::iterator it = record_v.begin();
           it != record_v.end(); it++) {
        DBRecord& record(*it);
        int id = record.getInt("id");
        if (id > 0) {
          info = LoggerInfo(record.get("node"), record.get("table"), id);
        }
      }
    } catch (const DBHandlerException& e) {
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
  return info;
}

int LoggerInfoTable::add(const LoggerInfo& info)
{
  if (m_db != NULL) {
    try {
      if (info.getId() == 0) {
        m_db->execute("select addlogger as id from "
                      "addlogger('%s', '%s');",
                      info.getNode().c_str(),
                      info.getTable().c_str());
      } else {
        m_db->execute("select addlogger as id from "
                      "addlogger('%s', '%s', %d);",
                      info.getNode().c_str(),
                      info.getTable().c_str(),
                      info.getId());
      }
      DBRecordList record_v(m_db->loadRecords());
      if (record_v.size() > 0) return record_v[0].getInt("id");
    } catch (const DBHandlerException& e) {
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
  return 0;
}
