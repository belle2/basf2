#include <daq/slc/psql/PostgreSQLInterface.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("usage: %s <dbtabe> <config>", argv[0]);
    return 1;
  }
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  db.connect();
  try {
    db.execute("delete from %s where path like '.%s.'", argv[1], argv[2]);
    db.execute("delete from %s where path like '.%s._%%'", argv[1], argv[2]);
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
  }
  db.close();
  return 0;
}
