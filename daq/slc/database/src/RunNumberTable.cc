#include "daq/slc/database/RunNumberTable.h"

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <sstream>

using namespace Belle2;

RunNumber RunNumberTable::add(const std::string& config,
                              int expno, int runno,
                              int subno, bool isstart)
{
  try {
    if (expno <= 0) expno = getExpNumber();
    if (expno < 0) {
      return RunNumber();
    }
    if (runno <= 0) runno = getRunNumber(expno);
    if (runno < 0) {
      return RunNumber();
    }
    m_db.execute("insert into runnumber (label) "
                 "values ('%s.%c.%03d.%06d.%03d') "
                 "returning id, extract(epoch from record_time);",
                 config.c_str(), (isstart ? 's' : 'e'), expno, runno, subno);
    DBRecordList record_v(m_db.loadRecords());
    if (record_v.size() > 0) {
      int id = record_v[0].getInt("id");
      long long record_time = record_v[0].getInt("record_time");
      return RunNumber(config, expno, runno, subno, id, record_time);
    }
  } catch (const DBHandlerException& e) {
    LogFile::error("DB access error : %s", e.what());
  }
  return RunNumber();
}

RunNumber RunNumberTable::add(const RunNumber& rn)
{
  return add(rn.getConfig(), rn.getExpNumber(),
             rn.getRunNumber(), rn.getSubNumber(), rn.isStart());
}

int RunNumberTable::getRunNumber(int expno, const std::string& config)
{
  if (expno <= 0) return -1;
  int runno = 1;
  try {
    if (config.size() == 0) {
      m_db.execute("select max(cast(split_part(label,'.',4) as int))+1 "
                   "as runno from runnumber "
                   "where label like '_%%.%03d._%%' limit 1;", expno);
    } else {
      m_db.execute("select max(cast(split_part(label,'.',4) as int))+1 "
                   "as runno from runnumber "
                   "where label like '_%s.s.%03d._%%' limit 1;",
                   config.c_str(), expno);
    }
    DBRecordList record_v(m_db.loadRecords());
    if (record_v.size() > 0) {
      runno = record_v[0].getInt("runno");
      if (runno <= 0) runno = 1;
    }
  } catch (const DBHandlerException& e) {
    LogFile::error("DB access error : %s", e.what());
    return -1;
  }
  return runno;
}

int RunNumberTable::getExpNumber(const std::string& config)
{
  int expno = 1;
  try {
    if (config.size() == 0) {
      m_db.execute("select max(cast(split_part(label,'.',3) as int))"
                   " as expno from runnumber;");
    } else {
      m_db.execute("select max(cast(split_part(label,'.',3) as int))"
                   " as expno from runnumber "
                   "where label like '_%s.s._%%._%%' limit 1;", config.c_str());
    }
    DBRecordList record_v(m_db.loadRecords());
    if (record_v.size() > 0) {
      expno = record_v[0].getInt("expno");
      if (expno <= 0) expno = 1;
    }
  } catch (const DBHandlerException& e) {
    LogFile::error("DB access error : %s", e.what());
    return -1;
  }
  return expno;
}
