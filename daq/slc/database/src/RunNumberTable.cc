#include "daq/slc/database/RunNumberTable.h"

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <sstream>
#include <iostream>

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
    m_db.execute("insert into runnumber (config, expno, runno, subno, isstart) "
                 "values ('%s', %d, %d, %d, %s) "
                 "returning id, extract(epoch from record_time) as tstart;",
                 config.c_str(), expno, runno, subno, (isstart ? "true" : "false"));
    DBRecordList record_v(m_db.loadRecords());
    if (record_v.size() > 0) {
      int id = record_v[0].getInt("id");
      int record_time = record_v[0].getInt("tstart");
      return RunNumber(config, expno, runno, subno, isstart, id, record_time);
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

int RunNumberTable::getRunNumber(int expno)
{
  if (expno <= 0) return -1;
  int runno = 1;
  try {
    if (expno == 0) {
      m_db.execute("select max(runno)+1 as runno from runnumber "
                   "limit 1;");
    } else {
      m_db.execute("select max(runno)+1 as runno from runnumber "
                   "where expno = %d limit 1;", expno);
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
      m_db.execute("select max(expno) as expno from runnumber limit 1;");
    } else {
      m_db.execute("select max(expno) as expno from runnumber "
                   "where config = '%s' limit 1;", config.c_str());
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

RunNumberList RunNumberTable::get(int expno, bool started, int runno_min, int runno_max)
{
  RunNumberList list;
  try {
    const char* cmd = "select config, expno, runno, subno, isstart, id, "
                      "extract(epoch from record_time) as record_time from runnumber";
    if (expno == 0) {
      if (runno_max > 0) {
        m_db.execute("%s where isstart = %s and runno >= %d and runno <= %d",
                     cmd, started, runno_min, runno_max);
      } else {
        m_db.execute("%s where isstart = %s and runno >= %d", cmd, started, runno_min);
      }
    } else {
      if (runno_max > 0) {
        m_db.execute("%s where expno = %d and isstart = %s and runno >= %d and runno <= %d",
                     cmd, expno, started, runno_min, runno_max);
      } else {
        m_db.execute("%s where expno = %d and isstart = %s and runno >= %d",
                     cmd, expno, started, runno_min);
      }
    }
    const DBRecordList record_v(m_db.loadRecords());
    for (DBRecordList::const_iterator it = record_v.begin();
         it != record_v.end(); it++) {
      const DBRecord& record(*it);
      list.push_back(RunNumber(record.get("config"), record.getInt("expno"),
                               record.getInt("runno"), record.getInt("subno"),
                               record.getInt("id"), record.getInt("record_time")));
    }
  } catch (const DBHandlerException& e) {
    LogFile::error("DB access error : %s", e.what());
  }
  return list;
}

RunNumberList RunNumberTable::get(int expno, int runno_min, int runno_max)
{
  RunNumberList list;
  try {
    const char* cmd = "select config, expno, runno, subno, isstart, id, "
                      "extract(epoch from record_time) as record_time from runnumber";
    if (expno == 0) {
      if (runno_max > 0) {
        m_db.execute("%s where runno >= %d and runno <= %d", cmd, runno_min, runno_max);
      } else {
        m_db.execute("%s where runno >= %d", cmd, runno_min);
      }
    } else {
      if (runno_max > 0) {
        m_db.execute("%s where expno = %d and runno >= %d and runno <= %d",
                     cmd, expno, runno_min, runno_max);
      } else {
        m_db.execute("%s where expno = %d and runno >= %d", cmd, expno, runno_min);
      }
    }
    const DBRecordList record_v(m_db.loadRecords());
    for (DBRecordList::const_iterator it = record_v.begin();
         it != record_v.end(); it++) {
      const DBRecord& record(*it);
      list.push_back(RunNumber(record.get("config"), record.getInt("expno"),
                               record.getInt("runno"), record.getInt("subno"),
                               record.getBool("isstart"), record.getInt("id"),
                               record.getInt("record_time")));
    }
  } catch (const DBHandlerException& e) {
    LogFile::error("DB access error : %s", e.what());
  }
  return list;
}

RunNumberList RunNumberTable::get(const std::string& type, int expno, int runno_min, int runno_max)
{
  RunNumberList list;
  try {
    std::stringstream ss;
    ss << "select config, expno, runno, subno, isstart, id, "
       << "extract(epoch from record_time) as record_time from runnumber "
       << "where (config like '%%_" << type << "%%_' or config like '%%_"
       << type << "' or config like '" << type << "%%_') and";
    const std::string cmd = ss.str();
    if (expno == 0) {
      if (runno_max > 0) {
        m_db.execute("%s runno >= %d and runno <= %d", cmd.c_str(), runno_min, runno_max);
      } else {
        m_db.execute("%s runno >= %d", cmd.c_str(), runno_min);
      }
    } else {
      if (runno_max > 0) {
        m_db.execute("%s expno = %d and runno >= %d and runno <= %d",
                     cmd.c_str(), expno, runno_min, runno_max);
      } else {
        m_db.execute("%s expno = %d and runno >= %d", cmd.c_str(), expno, runno_min);
      }
    }
    const DBRecordList record_v(m_db.loadRecords());
    for (DBRecordList::const_iterator it = record_v.begin();
         it != record_v.end(); it++) {
      const DBRecord& record(*it);
      list.push_back(RunNumber(record.get("config"), record.getInt("expno"),
                               record.getInt("runno"), record.getInt("subno"),
                               record.getBool("isstart"), record.getInt("id"),
                               record.getInt("record_time")));
    }
  } catch (const DBHandlerException& e) {
    LogFile::error("DB access error : %s", e.what());
  }
  return list;
}

void RunNumberTable::create()
{
  if (!m_db.checkTable("runnumber")) {
    m_db.execute("create table runnumber ("
                 "record_time  timestamp with time zone "
                 "not null default current_timestamp, "
                 "id serial not null primary key, "
                 "config text not null, "
                 "expno int not null, "
                 "runno int not null, "
                 "subno int not null, "
                 "isstart boolean not null, "
                 "unique (expno, runno, subno, isstart));");
    m_db.execute("create index runnumber_id_index on runnumber(id);");
  }
}
