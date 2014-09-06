#include "daq/slc/database/RunNumberInfoTable.h"

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <sstream>

using namespace Belle2;

RunNumberInfo RunNumberInfoTable::add(int expno, int runno, int subno)
{
  if (m_db->checkTable("runnumberinfo")) {
    try {
      if (expno <= 0) expno = getExpNumber();
      if (runno <= 0) runno = getRunNumber(expno);
      m_db->execute("select id, expno, runno, subno, "
                    "extract(epoch from record_time) as record_time "
                    "from addrunnumber(%d, %d, %d, true);",
                    expno, runno, subno);
      DBRecordList record_v(m_db->loadRecords());
      if (record_v.size() > 0) {
        return RunNumberInfo(record_v[0].getInt("expno"),
                             record_v[0].getInt("runno"),
                             record_v[0].getInt("subno"),
                             record_v[0].getInt("id"),
                             record_v[0].getInt("record_time"));
      }
    } catch (const DBHandlerException& e) {
      //LogFile::error("DB access error : %s", e.what());
    }
  }
  return RunNumberInfo();
}

RunNumberInfo RunNumberInfoTable::add(const RunNumberInfo& info)
{
  if (m_db->checkTable("runnumberinfo")) {
    try {
      int expno = info.getExpNumber();
      int runno = info.getRunNumber();
      int subno = info.getSubNumber();
      m_db->execute("select id, expno, runno, subno, "
                    "extract(epoch from record_time) as record_time "
                    "from addrunnumber(%d, %d, %d, false);",
                    expno, runno, subno);
      DBRecordList record_v(m_db->loadRecords());
      if (record_v.size() > 0) {
        return RunNumberInfo(record_v[0].getInt("expno"),
                             record_v[0].getInt("runno"),
                             record_v[0].getInt("subno"),
                             record_v[0].getInt("id"),
                             record_v[0].getInt("record_time"));
      }
    } catch (const DBHandlerException& e) {
      //LogFile::error("DB access error : %s", e.what());
    }
  }
  return RunNumberInfo();
}

RunNumberInfoList RunNumberInfoTable::getList(int expno, int runno, int subno, int max)
{
  RunNumberInfoList info_v;
  if (m_db->checkTable("runnumberinfo")) {
    try {
      std::stringstream ss;
      if (expno > 0) ss << " and expno = " << expno;
      if (runno > 0) ss << " and runno = " << runno;
      if (subno > 0) ss << " and subno = " << subno;
      m_db->execute("select id, expno, runno, subno from runnumberinfo "
                    "where %s order by id desc, expno desc, runno desc, "
                    "subno desc %s;", ss.str().c_str(),
                    ((max > 0) ? StringUtil::form("limit %d ", max).c_str() : ""));
      DBRecordList record_v(m_db->loadRecords());
      for (size_t i = 0; i < record_v.size(); i++) {
        DBRecord& record(record_v[i]);
        info_v.push_back(RunNumberInfo(record.getInt("expno"), record.getInt("runno"),
                                       record.getInt("subno"), record.getInt("id")));
      }
    } catch (const DBHandlerException& e) {
      //LogFile::error("DB access error : %s", e.what());
    }
  }
  return info_v;
}

int RunNumberInfoTable::getRunNumber(int expno)
{
  int runno = 1;
  if (m_db->checkTable("runnumberinfo")) {
    try {
      std::stringstream ss;
      if (expno > 0) {
        ss << "expno = " << expno;
      } else {
        ss << "expno = (select max(expno) from runnumberifno)";
      }
      m_db->execute("select max(runno)+1 as runno from runnumberinfo "
                    "where %s limit 1;", ss.str().c_str());
      DBRecordList record_v(m_db->loadRecords());
      if (record_v.size() > 0) {
        runno = record_v[0].getInt("runno");
        if (runno <= 0) runno = 1;
      }
    } catch (const DBHandlerException& e) {
      //LogFile::error("DB access error : %s", e.what());
    }
  }
  return runno;
}

int RunNumberInfoTable::getExpNumber()
{
  int expno = 1;
  if (m_db->checkTable("runnumberinfo")) {
    try {
      m_db->execute("select max(expno) as expno from runnumberinfo limit 1;");
      DBRecordList record_v(m_db->loadRecords());
      if (record_v.size() > 0) {
        expno = record_v[0].getInt("expno");
        if (expno <= 0) expno = 1;
      }
    } catch (const DBHandlerException& e) {
      //LogFile::error("DB access error : %s", e.what());
    }
  }
  return expno;
}
