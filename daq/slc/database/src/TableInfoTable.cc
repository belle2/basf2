#include "daq/slc/database/TableInfoTable.h"

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

TableInfoList TableInfoTable::getList() throw()
{
  TableInfoList info_v;
  if (m_db != NULL) {
    try {
      m_db->execute("select name, isconfig, revision, isroot, id from tableinfo;");
      DBRecordList record_v(m_db->loadRecords());
      for (DBRecordList::iterator it = record_v.begin();
           it != record_v.end(); it++) {
        DBRecord& record(*it);
        info_v.push_back(TableInfo(record.get("name"), record.getBool("isconfig"),
                                   record.getInt("revision"), record.getBool("isroot"),
                                   record.getInt("id")));
      }
    } catch (const DBHandlerException& e) {
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
  return info_v;
}

TableInfo TableInfoTable::get(const std::string& tablename, int revision)
throw()
{
  TableInfo info;
  if (m_db != NULL) {
    try {
      m_db->execute("select * from tableinfo "
                    "where name = '%s' and revision =%d;",
                    tablename.c_str(), revision);
      DBRecordList record_v(m_db->loadRecords());
      if (record_v.size() > 0) {
        DBRecord& record(record_v[0]);
        info = TableInfo(record.get("name"), record.getBool("isconfig"),
                         record.getInt("revision"), record.getBool("isroot"),
                         record.getInt("id"));
      }
    } catch (const DBHandlerException& e) {
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
  return info;
}

int TableInfoTable::add(const TableInfo& info) throw()
{
  if (m_db != NULL) {
    try {
      if (info.getRevision() <= 0) {
        m_db->execute("select addtable as id from addtable('%s', 1, %s, %s);",
                      info.getName().c_str(), (info.isConfig() ? "true" : "false"),
                      (info.isRoot() ? "true" : "false"));
      } else {
        m_db->execute("select addtable as id from addtable('%s', %d, %s, %s);",
                      info.getName().c_str(), info.getRevision(),
                      (info.isConfig() ? "true" : "false"),
                      (info.isRoot() ? "true" : "false"));
      }
      DBRecordList record_v(m_db->loadRecords());
      if (record_v.size() > 0) return record_v[0].getInt("id");
    } catch (const DBHandlerException& e) {
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
  return 0;
}

