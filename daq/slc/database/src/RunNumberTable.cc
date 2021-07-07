/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/database/RunNumberTable.h"

#include <daq/slc/database/DBHandlerException.h>
#include <daq/slc/database/DBInterface.h>

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

RunNumber RunNumberTable::add(const std::string& node,
                              const std::string& runtype,
                              int expno, int runno)
{
  if (runtype.size() == 0 || node.size() == 0) {
    LogFile::fatal("Empty node name : node='%s' runtype='%s'", node.c_str(), runtype.c_str());
    return RunNumber();
  }
  if (runtype.size() == 0 || node.size() == 0) {
    LogFile::fatal("Empty runtype : node='%s' runtype='%s'", node.c_str(), runtype.c_str());
    return RunNumber();
  }
  if (node == "pxd" || node == "svd" || node == "cdc" || node == "top" ||
      node == "arich" || node == "ecl" || node == "klm" || node == "trg" || node == "b4test") {
    if (node != runtype) {
      LogFile::fatal("wrong node name or runtype : node='%s' != runtype='%s'", node.c_str(), runtype.c_str());
      return RunNumber();
    }
  } else if (node == "global") {
    if (runtype != "physics" && runtype != "cosmic" && runtype != "beam" &&
        runtype != "null" && runtype != "debug" && runtype != "hlttest") {
      LogFile::fatal("wrong node name or runtype : node='%s' runtype='%s'!=physics/cosmic/beam/debug/hlttest/null", node.c_str(),
                     runtype.c_str());
      return RunNumber();
    }
  } else {
    LogFile::fatal("wrong node name : node='%s'!=global/pxd/svd/cdc/top/arich/ecl/klm/trg/b4test runtype='%s'", node.c_str(),
                   runtype.c_str());
  }
  try {
    int expno_tmp = getExpNumber("");
    if (node != "global" || expno < expno_tmp) expno = expno_tmp;
    int runno_tmp = getRunNumber(node, expno);
    if (runno < runno_tmp) runno = runno_tmp;
    m_db.execute("insert into runnumber_new (node, runtype, expno, runno) "
                 "values ('%s', '%s', %d, %d) "
                 "returning id, extract(epoch from record_time) as tstart;",
                 node.c_str(), runtype.c_str(), expno, runno);
    DBRecordList record_v(m_db.loadRecords());
    if (record_v.size() > 0) {
      int id = record_v[0].getInt("id");
      int record_time = record_v[0].getInt("tstart");
      return RunNumber(node, runtype, expno, runno, id, record_time);
    }
  } catch (const DBHandlerException& e) {
    LogFile::error("DB access error : %s", e.what());
  }
  return RunNumber();
}

RunNumber RunNumberTable::add(const RunNumber& rn)
{
  return add(rn.getNode(), rn.getRunType(), rn.getExpNumber(), rn.getRunNumber());
}

int RunNumberTable::getRunNumber(const std::string& node, int expno)
{
  if (expno <= 0) return -1;
  int runno = 1;
  try {
    //if (node.size() == 0) {
    if (expno == 0) {
      m_db.execute("select max(runno)+1 as runno from runnumber_new "
                   "limit 1;");
    } else {
      m_db.execute("select max(runno)+1 as runno from runnumber_new "
                   "where expno = %d limit 1;", expno);
    }
    /*
    } else {
    if (expno == 0) {
    m_db.execute("select max(runno)+1 as runno from runnumber_new "
    "where node = '%s' limit 1;", node.c_str());
    } else {
    m_db.execute("select max(runno)+1 as runno from runnumber_new "
    "where node = '%s' and expno = %d limit 1;",
    node.c_str(), expno);
    }
    }
    */
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

int RunNumberTable::getExpNumber(const std::string& node)
{
  int expno = 1;
  try {
    if (node.size() == 0) {
      m_db.execute("select max(expno) as expno from runnumber_new limit 1;");
    } else {
      m_db.execute("select max(expno) as expno from runnumber_new "
                   "where node = '%s' limit 1;", node.c_str());
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

RunNumberList RunNumberTable::get(const std::string& node, int expno, int runno_min, int runno_max)
{
  RunNumberList list;
  try {
    const char* cmd = "select node, runtype, expno, runno, id, "
                      "extract(epoch from record_time) as record_time from runnumber_new";
    if (node.size() == 0) {
      if (expno == 0) {
        if (runno_max > 0) {
          m_db.execute("%s where runno >= %d and runno <= %d order by id", cmd, runno_min, runno_max);
        } else {
          m_db.execute("%s where runno >= %d order by id", cmd, runno_min);
        }
      } else {
        if (runno_max > 0) {
          m_db.execute("%s where expno = %d and runno >= %d and runno <= %d order by id",
                       cmd, expno, runno_min, runno_max);
        } else {
          m_db.execute("%s where expno = %d and runno >= %d order by id", cmd, expno, runno_min);
        }
      }
    } else {
      if (expno == 0) {
        if (runno_max > 0) {
          m_db.execute("%s where node = '%s' and runno >= %d and runno <= %d order by id",
                       cmd, node.c_str(), runno_min, runno_max);
        } else {
          m_db.execute("%s where node = '%s' and runno >= %d order by id", cmd, node.c_str(), runno_min);
        }
      } else {
        if (runno_max > 0) {
          m_db.execute("%s where node = '%s' and expno = %d and runno >= %d and runno <= %d order by id",
                       cmd, node.c_str(), expno, runno_min, runno_max);
        } else {
          m_db.execute("%s where node = '%s' and expno = %d and runno >= %d order by id",
                       cmd, node.c_str(), expno, runno_min);
        }
      }
    }
    const DBRecordList record_v(m_db.loadRecords());
    for (DBRecordList::const_iterator it = record_v.begin();
         it != record_v.end(); it++) {
      const DBRecord& record(*it);
      list.push_back(RunNumber(record.get("node"), record.get("runtype"),
                               record.getInt("expno"), record.getInt("runno"),
                               record.getInt("id"), record.getInt("record_time")));
    }
  } catch (const DBHandlerException& e) {
    LogFile::error("DB access error : %s", e.what());
  }
  return list;
}

RunNumberList RunNumberTable::get(int expno, int runno_min, int runno_max)
{
  return get("", expno, runno_min, runno_max);
}

void RunNumberTable::create()
{
  if (!m_db.checkTable("runnumber_new")) {
    m_db.execute("create table runnumber_new ("
                 "record_time  timestamp with time zone "
                 "not null default current_timestamp, "
                 "id serial not null primary key, "
                 "node text not null, "
                 "runtype text not null, "
                 "expno int not null, "
                 "runno int not null, "
                 "unique (node, expno, runno));");
    m_db.execute("create index runnumber_new_id_index on runnumber_new(id);");
  }
}
