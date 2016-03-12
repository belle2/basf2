#include <daq/slc/database/PostgreSQLInterface.h>

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
  if (argc < 3 || strcmp(argv[1], "-help") == 0) {
    printf("%s : id filepath \n", argv[0]);
    return 1;
  }
  int id = atoi(argv[1]);
  const std::string filepath = argv[2];
  bool failed = std::string(argv[2]) == "-f";
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  std::stringstream ss;
  if (!failed) {
    ss << "update " << g_table << " set time_convert = current_timestamp, path_cc = '"
       << filepath << "' where id = " << id << " and time_convert is null;";
    try {
      db.connect();
      db.execute(ss.str().c_str());
      db.close();
    } catch (const DBHandlerException& e) {
      LogFile::error("Failed to update : %s", e.what());
      return 1;
    }
  } else {
    ss << "update " << g_table << " set time_process = null where id = " << id << " and time_convert is null;";
    try {
      db.connect();
      db.execute(ss.str().c_str());
      db.close();
    } catch (const DBHandlerException& e) {
      LogFile::error("Failed to update : %s", e.what());
      return 1;
    }
  }
  return 0;
}

