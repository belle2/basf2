#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/MonitorDB.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>
#include <cstdlib>

using namespace Belle2;

int main(int argc, const char** argv)
{
  if (argc < 3) {
    LogFile::debug("usage: %s <tablename> <vname>", argv[0]);
    return 1;
  }
  const std::string tablename = argv[1];
  const std::string vname = argv[2];
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  NSMVarList vars(MonitorDB::get(db, tablename, vname));
  int index = 0;
  for (NSMVarList::iterator i = vars.begin();
       i != vars.end(); i++) {
    NSMVar& var(*i);
    if (var.getType() == NSMVar::TEXT) {
      LogFile::debug("%s[%d] = %s (time=%s)", var.getName().c_str(),
                     index, var.getText(), Date(var.getDate()).toString());
    } else if (var.getType() == NSMVar::INT) {
      LogFile::debug("%s[%d] = %d (time=%s)", var.getName().c_str(),
                     index, var.getInt(), Date(var.getDate()).toString());
    } else if (var.getType() == NSMVar::FLOAT) {
      LogFile::debug("%s[%d] = %f (time=%s)", var.getName().c_str(),
                     index, var.getFloat(), Date(var.getDate()).toString());
    } else {
    }
    index++;
  }
  return 0;
}
