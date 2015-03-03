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
  if (argc < 2) {
    LogFile::debug("usage: %s <tablename> [<nodename>] [<grep>]", argv[0]);
    return 1;
  }
  const std::string tablename = argv[1];
  const std::string nodename = (argc > 2) ? argv[2] : "";
  const std::string grep = (argc > 3) ? argv[3] : "";
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  StringList list = DBObjectLoader::getDBlist(db, tablename, nodename, grep);
  for (size_t i = 0; i < list.size(); i++) {
    std::cout << list[i] << std::endl;
  }
  /*
  db.connect();
  if (argc < 3) {
    db.execute("select name from %s where name = REPLACE(path, '.', '');",
               tablename.c_str());
  } else {
    db.execute("select name from %s where name = REPLACE(path, '.', '') and name like '%s@_%';",
               tablename.c_str(), nodename.c_str());
  }
  DBRecordList record_v(db.loadRecords());
  for (size_t i = 0; i < record_v.size(); i++) {
    DBRecord& record(record_v[i]);
    std::cout << record.get("name") << std::endl;
  }
  */
  return 0;
}

