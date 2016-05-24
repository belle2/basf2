#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <cstring>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("usage: %s <tablename> <configname> [--all] [--html]", argv[0]);
    return 1;
  }
  const std::string tablename = argv[1];
  const std::string configname = argv[2];
  bool showall = false;
  bool showhtml = false;
  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "--all") == 0) {
      showall = true;
    } else if (strcmp(argv[i], "--html") == 0) {
      showhtml = true;
    }
  }
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  DBObject obj = DBObjectLoader::load(db, tablename, configname, showall);
  if (showhtml) {
    obj.printHTML(showall);
  } else {
    obj.print(showall);
  }
  return 0;
}
