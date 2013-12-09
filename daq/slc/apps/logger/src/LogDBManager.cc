#include "daq/slc/apps/logger/LogDBManager.h"

#include "daq/slc/base/StringUtil.h"

using namespace Belle2;

void LogDBManager::createTable() throw(DBHandlerException)
{
  SystemLog log;
  _db->execute(Belle2::form("create table \"%s_log\" (%s);",
                            _tablename.c_str(), log.toSQLConfig().c_str()));
}

void LogDBManager::writeLog(const SystemLog& log) throw(DBHandlerException)
{
  _db->execute(Belle2::form("insert into \"%s_log\" (%s) values (%s);",
                            _tablename.c_str(), log.toSQLNames().c_str(),
                            log.toSQLValues().c_str()));
}

std::vector<SystemLog> LogDBManager::readLogs(const std::string& groupname,
                                              const std::string& hostname,
                                              const std::string& nodename,
                                              int days, int hours, int mins)
throw(DBHandlerException)
{
  std::string opt = "";
  if (groupname != "" && groupname != "ALL") {
    opt += Belle2::form("group = \"%s\"", groupname.c_str());
  }
  if (hostname != "" && hostname != "ALL") {
    if (opt.size() > 0) opt += " and ";
    opt += Belle2::form("host = \"%s\"", hostname.c_str());
  }
  if (nodename != "" && nodename != "ALL") {
    if (opt.size() > 0) opt += " and ";
    opt += Belle2::form("node = \"%s\"", nodename.c_str());
  }
  if (days >= 0 && hours >= 0 && mins >= 0) {
    if (opt.size() > 0) opt += " and ";
    int threshold = Date().get() - (((days * 24 + hours) * 60 + mins) * 60);
    opt += Belle2::form("log_date > %d", threshold);
  }
  _mutex.lock();
  try {
    _db->execute(Belle2::form("selct * from \"%s_log\" %s;",
                              _tablename.c_str(), opt.c_str()));
  } catch (const DBHandlerException& e) {
    _mutex.unlock();
    throw (e);
  }
  DBRecordList& record_v(_db->loadRecords());
  std::vector<SystemLog> log_v;
  for (size_t i = 0; i < record_v.size(); i++) {
    SystemLog log(record_v[i].getFieldValue("node"),
                  (SystemLog::Priority)record_v[i].getFieldValueInt("priority"),
                  record_v[i].getFieldValue("message"),
                  record_v[i].getFieldValueInt("ref_no"));
    log.setDate(record_v[i].getFieldValueInt("log_date"));
    log.setGroupName(record_v[i].getFieldValue("group"));
    log.setHostName(record_v[i].getFieldValue("host"));
    log_v.push_back(log);
  }
  _mutex.lock();
  return log_v;
}

