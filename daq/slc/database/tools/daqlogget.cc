#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/DAQLogDB.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("usage: %s <tablename> [-n <nodename>] [-m <max>] [-b begin_date] [-e end_date]", argv[0]);
    return 1;
  }
  const std::string tablename = argv[1];
  std::string nodename;
  int max = 0;
  std::stringstream ss_begin;
  std::stringstream ss_end;
  bool colored = true;
  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "-m") == 0) {
      i++;
      if (i < argc) max = atoi(argv[i]);
    }
    if (strcmp(argv[i], "-nc") == 0) {
      colored = false;
    }
    if (strcmp(argv[i], "-n") == 0) {
      i++;
      if (i < argc) nodename = argv[i];
    }
    if (strcmp(argv[i], "-b") == 0) {
      i++;
      if (i < argc - 1) {
        ss_begin << argv[i] << " ";
        i++;
        ss_begin << argv[i];
      }
    }
    if (strcmp(argv[i], "-e") == 0) {
      i++;
      if (i < argc - 1) {
        ss_end << argv[i] << " ";
        i++;
        ss_end << argv[i];
      }
    }
  }
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  DAQLogMessageList logs = DAQLogDB::getLogs(db, tablename, nodename,
                                             ss_begin.str(), ss_end.str(), max);
  for (size_t i = 0; i < logs.size(); i++) {
    if (colored) {
      switch (logs[i].getPriority()) {
        case LogFile::DEBUG:   std::cout << "\x1b[49m\x1b[39m"; break;
        case LogFile::INFO:    std::cout << "\x1b[49m\x1b[32m"; break;
        case LogFile::NOTICE:  std::cout << "\x1b[49m\x1b[34m"; break;
        case LogFile::WARNING: std::cout << "\x1b[49m\x1b[35m"; break;
        case LogFile::ERROR:   std::cout << "\x1b[49m\x1b[31m"; break;
        case LogFile::FATAL:   std::cout << "\x1b[41m\x1b[37m"; break;
        default: break;
      }
    }
    std::cout << "[" << logs[i].getNodeName() << "] ["
              << logs[i].getDate().toString() << "] ["
              << logs[i].getPriorityText() << "] "
              << logs[i].getMessage();
    if (colored) {
      std::cout << "\x1b[49m\x1b[39m";
    }
    std::cout << std::endl;
  }
  return 0;
}

