#include <daq/slc/database/PostgreSQLInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <sstream>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("usage: %s <tablename> <configname> [all]", argv[0]);
    return 1;
  }
  const std::string tablename = argv[1];
  const std::string configname = argv[2];
  bool showall = (argc > 3 && StringUtil::tolower(argv[3]) == "all");
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  DBObject obj = DBObjectLoader::load(db, tablename, configname, showall);
  obj.print(showall);
  return 0;
}
