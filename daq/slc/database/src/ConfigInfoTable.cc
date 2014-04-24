#include "daq/slc/database/ConfigInfoTable.h"

#include <daq/slc/base/StringUtil.h>

#include <daq/slc/system/LogFile.h>

#include <sstream>
#include <iostream>

using namespace Belle2;

const std::string ConfigInfo::getSQL() const throw()
{
  return StringUtil::form("select configid from configid('%s', '%s', '%s', %d)",
                          getName().c_str(), getNode().c_str(),
                          getTable().c_str(), getRevision());
}

ConfigInfoList ConfigInfoTable::getList()
{
  ConfigInfoList info_v;
  if (m_db != NULL) {
    try {
      m_db->execute("select * from confignames();");
      DBRecordList record_v(m_db->loadRecords());
      for (DBRecordList::iterator it = record_v.begin();
           it != record_v.end(); it++) {
        DBRecord& record(*it);
        int id = record.getInt("id");
        if (id > 0) {
          info_v.push_back(ConfigInfo(record.get("name"),
                                      record.get("node"),
                                      record.get("table"),
                                      record.getInt("revision"), id));
        }
      }
    } catch (const DBHandlerException& e) {
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
  return info_v;
}

ConfigInfoList ConfigInfoTable::getList(const std::string& nodename)
{
  ConfigInfoList info_v;
  if (m_db != NULL) {
    try {
      m_db->execute("select * from confignames('%s')", nodename.c_str());
      DBRecordList record_v(m_db->loadRecords());
      for (DBRecordList::iterator it = record_v.begin();
           it != record_v.end(); it++) {
        DBRecord& record(*it);
        int id = record.getInt("id");
        if (id > 0) {
          info_v.push_back(ConfigInfo(record.get("name"),
                                      record.get("node"),
                                      record.get("table"),
                                      record.getInt("revision"), id));
        }
      }
    } catch (const DBHandlerException& e) {
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
  return info_v;
}

ConfigInfo ConfigInfoTable::get(int configid)
{
  ConfigInfo info;
  if (configid > 0 && m_db != NULL) {
    try {
      std::stringstream ss;
      ss << "select name, id, "
         "(select name from nodeinfo where id = nodeid) as node, "
         "(select name from tableinfo where id = tableid) as table "
         "from configinfo where id = " << configid << ";";
      m_db->execute(ss.str());
      DBRecordList record_v(m_db->loadRecords());
      for (DBRecordList::iterator it = record_v.begin();
           it != record_v.end(); it++) {
        DBRecord& record(*it);
        int id = record.getInt("id");
        if (id > 0) {
          info = ConfigInfo(record.get("name"),
                            record.get("node"),
                            record.get("table"),
                            record.getInt("revision"), id);
        }
      }
    } catch (const DBHandlerException& e) {
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
  return info;
}

void ConfigInfoTable::add(const ConfigInfo& info)
{
  if (m_db != NULL) {
    try {
      if (info.getName().size() > 0) {
        m_db->execute("select addconfig as id from "
                      "addconfig('%s', '%s', '%s', %d);",
                      info.getName().c_str(),
                      info.getNode().c_str(),
                      info.getTable().c_str(),
                      info.getRevision());
      }
    } catch (const DBHandlerException& e) {
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
}
