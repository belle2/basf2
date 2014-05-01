#include "daq/slc/database/TableInfoTable.h"

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <fstream>
#include <iostream>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 1) {
    LogFile::debug("usage: %s", argv[0]);
    return 1;
  }
  ConfigFile dbconfig("slowcontrol");
  DBInterface* db = new PostgreSQLInterface(dbconfig.get("database.host"),
                                            dbconfig.get("database.dbname"),
                                            dbconfig.get("database.user"),
                                            dbconfig.get("database.password"),
                                            dbconfig.getInt("database.port"));
  db->connect();
  TableInfoList info_v = TableInfoTable(db).getList();
  for (size_t i = 0; i < info_v.size(); i++) {
    try {
      db->execute("drop table \"%s:%s:%d\"",
                  (info_v[i].isConfig() ? "configinfo" : "loggerinfo"),
                  info_v[i].getName().c_str(), info_v[i].getRevision());
    } catch (const DBHandlerException& e) {
    }
  }
  db->close();
  return 0;
}
