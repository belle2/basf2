#include <daq/slc/database/FieldInfoTable.h>
#include <daq/slc/database/PostgreSQLInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("usage: %s <tablename> <dir> [root]", argv[0]);
    return 1;
  }
  std::string path = argv[2];
  std::string tablename = argv[1];
  ConfigObject obj = DBObjectLoader::load(path, tablename);
  bool isroot = (tablename.find(".") == std::string::npos);
  ConfigFile dbconfig("slowcontrol");
  DBInterface* db = new PostgreSQLInterface(dbconfig.get("database.host"),
                                            dbconfig.get("database.dbname"),
                                            dbconfig.get("database.user"),
                                            dbconfig.get("database.password"),
                                            dbconfig.getInt("database.port"));
  db->connect();
  FieldInfoTable(db).createTable(obj, isroot);
  db->close();
  return 0;
}
