#include <daq/slc/psql/PostgreSQLInterface.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <sstream>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/statvfs.h>

using namespace Belle2;

const char* g_table = "fileinfo";

int main(int argc, char** argv)
{
  if (argc < 2 || strcmp(argv[1], "-help") == 0) {
    printf("%s : runno \n", argv[0]);
    return 1;
  }
  int runno = atoi(argv[1]);
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  std::stringstream ss;
  ss << "update " << g_table << " set time_runend = current_timestamp where runno = " << runno << " and time_runend is null;";
  try {
    db.connect();
    db.execute(ss.str().c_str());
    db.close();
  } catch (const DBHandlerException& e) {
    LogFile::error("Failed to update : %s", e.what());
    return 1;
  }

  return 0;
}

