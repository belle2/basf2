#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("usage: %s <filepath> <tablename>", argv[0]);
    return 1;
  }
  const std::string filename = argv[1];
  const std::string tablename = argv[2];
  DBObject obj = DBObjectLoader::load(filename);
  obj.print();
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  DBObjectLoader::createDB(db, tablename, obj);
  db.close();
  return 0;
}
