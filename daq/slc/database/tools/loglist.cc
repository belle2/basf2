#include <daq/slc/database/PostgreSQLInterface.h>
#include <daq/slc/database/DAQLogDB.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("usage: %s <tablename> [<nodename>] [<max>]", argv[0]);
    return 1;
  }
  const std::string tablename = argv[1];
  const std::string nodename = (argc > 2) ? StringUtil::toupper(argv[2]) : "";
  const int max = (argc > 3) ? atoi(argv[3]) : 0;
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  DAQLogMessageList logs = DAQLogDB::getLogs(db, tablename, nodename, max);
  for (size_t i = 0; i < logs.size(); i++) {
    switch (logs[i].getPriority()) {
      case LogFile::DEBUG:   std::cout << "\x1b[49m\x1b[39m"; break;
      case LogFile::INFO:    std::cout << "\x1b[49m\x1b[32m"; break;
      case LogFile::NOTICE:  std::cout << "\x1b[49m\x1b[34m"; break;
      case LogFile::WARNING: std::cout << "\x1b[49m\x1b[35m"; break;
      case LogFile::ERROR:   std::cout << "\x1b[49m\x1b[31m"; break;
      case LogFile::FATAL:   std::cout << "\x1b[41m\x1b[37m"; break;
      default: break;
    }
    std::cout << "[" << logs[i].getNodeName() << "] ["
              << logs[i].getDate().toString() << "] ["
              << logs[i].getPriorityText() << "] "
              << logs[i].getMessage() << "\x1b[49m\x1b[39m" << std::endl;
  }
  return 0;
}

