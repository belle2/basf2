#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <cstring>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  bool verbose = false;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      LogFile::debug("usage: %s <tablename> [-e expno] [-r runno] [-n <node>]", argv[0]);
      return 1;
    }
    if (strcmp(argv[i], "-v") == 0) {
      verbose = true;
    }
  }

  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  db.connect();
  db.execute("select * from configlist;");
  DBRecordList record_v(db.loadRecords());
  if (verbose) {
    std::cout << StringUtil::form("%10s", "tablename")
              << " : lastupdate" << std::endl;;
  }
  for (size_t i = 0; i < record_v.size(); i++) {
    DBRecord& record(record_v[i]);
    std::string name;
    if (record.hasField("name")) {
      name = record.get("name");
      if (verbose) {
        std::cout << StringUtil::form("%10s", name.c_str())
                  << " : " << record.get("lastupdate") << std::endl;;
      } else {
        std::cout << name << " ";
      }
    }
  }
  if (!verbose) {
    std::cout << std::endl;
  }
  return 0;
}

