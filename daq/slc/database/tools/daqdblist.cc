#include <daq/slc/database/PostgreSQLInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("usage: %s <tablename> [<prefix>]", argv[0]);
    return 1;
  }

  const std::string tablename = argv[1];
  const std::string prefix = (argc > 2) ? argv[2] : "";
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  StringList list = DBObjectLoader::getDBlist(db, tablename, prefix);
  for (size_t i = 0; i < list.size(); i++) {
    std::cout << list[i] << std::endl;
  }
  return 0;
}

