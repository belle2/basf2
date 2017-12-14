#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/MonitorDB.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>
#include <cstdlib>

using namespace Belle2;

int main(int argc, const char** argv)
{
  if (argc < 4) {
    LogFile::debug("usage: %s <tablename> <vname> <int/float/text> <val>", argv[0]);
    return 1;
  }
  const std::string tablename = argv[1];
  const std::string vname = argv[2];
  const std::string type = argv[3];
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  if (type == "text") {
    MonitorDB::add(db, tablename, vname, argv[4]);
  } else if (type == "int") {
    MonitorDB::add(db, tablename, vname, atoi(argv[4]));
  } else if (type == "float") {
    MonitorDB::add(db, tablename, vname, (float)atof(argv[4]));
  } else {
    LogFile::debug("usage: %s <tablename> <vname> <int/float/text> <val>", argv[0]);
    return 1;
  }
  return 0;
}
