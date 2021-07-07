/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/database/MonitorDB.h"

#include <daq/slc/database/DBHandlerException.h>
#include <daq/slc/system/LogFile.h>

using namespace Belle2;

void MonitorDB::add(DBInterface& db, const std::string& table,
                    const std::string& vname, int val)
{
  try {
    createTable(db, table);
    db.execute("insert into %s (name, value_i) values ('%s', %d);",
               table.c_str(), vname.c_str(), val);
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
    throw (e);
  }
}

void MonitorDB::add(DBInterface& db, const std::string& table,
                    const std::string& vname, float val)
{
  try {
    createTable(db, table);
    db.execute("insert into %s (name, value_f) values ('%s', %f);",
               table.c_str(), vname.c_str(), val);
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
    throw (e);
  }
}

void MonitorDB::add(DBInterface& db, const std::string& table,
                    const std::string& vname, const std::string& val)
{
  try {
    createTable(db, table);
    db.execute("insert into %s (name, value_t) values ('%s', '%s');",
               table.c_str(), vname.c_str(), val.c_str());
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
    throw (e);
  }

}

NSMVarList MonitorDB::get(DBInterface& db, const std::string& tablename,
                          const std::string& vname)
{
  if (!db.isConnected()) db.connect();
  db.execute("select *, extract(epoch from record_time) rtime "
             "from %s p where p.name = '%s' order by id;",
             tablename.c_str(), vname.c_str());
  return readTable(db, vname);
}

NSMVarList MonitorDB::get(DBInterface& db, const std::string& tablename,
                          const std::string& vname, int max)
{
  if (!db.isConnected()) db.connect();
  db.execute("select *, extract(epoch from record_time) rtime "
             "from %s p where p.name = '%s' limit %d order by id;",
             tablename.c_str(), vname.c_str(), max);
  return readTable(db, vname);
}

NSMVarList MonitorDB::get(DBInterface& db, const std::string& tablename,
                          const std::string& vname, const Date& start,
                          const Date& end)
{
  if (!db.isConnected()) db.connect();
  db.execute("select *, extract(epoch from record_time) rtime "
             "from %s p where p.name = '%s' and record_time > '%s' and "
             "record_time < '%s'order by id;", tablename.c_str(),
             vname.c_str(), start.toString(), end.toString());
  return readTable(db, vname);
}

NSMVarList MonitorDB::get(DBInterface& db, const std::string& tablename,
                          const std::string& vname, int max,
                          const Date& start, const Date& end)
{
  if (!db.isConnected()) db.connect();
  db.execute("select *, extract(epoch from record_time) rtime "
             "from %s p where p.name = '%s' and record_time > '%s' and "
             "record_time < '%s' limit %d order by id;",
             tablename.c_str(), vname.c_str(),
             start.toString(), end.toString(), max);
  return readTable(db, vname);
}

NSMVarList MonitorDB::readTable(DBInterface& db, const std::string& vname)
{
  DBRecordList record_v(db.loadRecords());
  NSMVarList vars;
  for (size_t i = 0; i < record_v.size(); i++) {
    DBRecord& record(record_v[i]);
    NSMVar var(vname);
    if (record.hasField("value_b")) {
      var = (int)record.getBool("value_b");
    } else if (record.hasField("value_c")) {
      var = (int)record.getInt("value_c");
    } else if (record.hasField("value_s")) {
      var = (int)record.getInt("value_s");
    } else if (record.hasField("value_i")) {
      var = record.getInt("value_i");
    } else if (record.hasField("value_f")) {
      var = record.getFloat("value_f");
    } else if (record.hasField("value_d")) {
      var = record.getFloat("value_d");
    } else if (record.hasField("value_t")) {
      var = record.get("value_t");
    }
    var.setId(record.getInt("id"));
    var.setDate(record.getInt("rtime"));
    LogFile::debug("%d=%s", record.getInt("rtime"), Date(var.getDate()).toString());
    vars.push_back(var);
  }
  return vars;
}

void MonitorDB::createTable(DBInterface& db, const std::string& tablename)
{
  if (!db.isConnected()) db.connect();
  if (!db.checkTable(tablename)) {
    db.execute("create table %s \n"
               "(name  varchar(64), \n"
               "id bigserial, \n"
               "record_time timestamp with time zone default current_timestamp, \n"
               "value_b boolean default NULL, \n"
               "value_c char default NULL, \n"
               "value_s smallint default NULL, \n"
               "value_i int default NULL, \n"
               "value_l bigint default NULL, \n"
               "value_f float default NULL, \n"
               "value_d double precision default NULL, \n"
               "value_t text default NULL \n"
               "); ", tablename.c_str());
    db.execute("create index %s_id_index on %s(id);",
               tablename.c_str(), tablename.c_str());
  }
}
