#include <daq/slc/database/PostgreSQLInterface.h>
#include <daq/slc/database/RunNumberTable.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("usage: %s <rcnode> <configname> [<expno>]", argv[0]);
    return 1;
  }
  const std::string rcnode = argv[1];
  const std::string configname = argv[2];
  int expno = (argc > 3) ? atoi(argv[3]) : 1;
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  db.connect();
  RunNumber rn(rcnode + "@" + configname, expno, 0, 0);
  rn = RunNumberTable(db).add(rn);
  LogFile::debug("runnumber : %03d.%05d.%03d",
                 rn.getExpNumber(), rn.getRunNumber(), rn.getSubNumber());
  return 0;
}
