#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("usage: %s <tablename> [-e expno] [-r runno] [-n <node>]", argv[0]);
    return 1;
  }

  const std::string tablename = argv[1];
  std::string nodename;
  int expno = -1;
  int runno = -1;
  bool runstart = false;
  bool runend = false;
  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "--end") == 0) {
      runend = true;
    } else if (strcmp(argv[i], "-e") == 0) {
      i++;
      if (i < argc) expno = atoi(argv[i]);
    }
    if (strcmp(argv[i], "-r") == 0) {
      i++;
      if (i < argc) runno = atoi(argv[i]);
    }
    if (strcmp(argv[i], "-n") == 0) {
      i++;
      if (i < argc) nodename = argv[i];
    }
    if (strcmp(argv[i], "--start") == 0) {
      runstart = true;
    }
  }
  std::stringstream prefix;
  if (expno >= 0) prefix << StringUtil::form("%04d:", expno);
  if (runno >= 0) prefix << StringUtil::form("%06d", runno);
  if ((!runstart && !runend) || (runstart && runend)) {
    prefix << ":_%";
  } else if (runstart) {
    prefix << ":s";
  } else if (runend) {
    prefix << ":e";
  }
  if (nodename.size() > 0) {
    if (prefix.str().size() > 0) {
      prefix << "=";
    }
    prefix << nodename;
  }

  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  StringList list = DBObjectLoader::getDBlist(db, tablename, prefix.str());
  for (size_t i = 0; i < list.size(); i++) {
    std::cout << list[i] << std::endl;
  }
  return 0;
}

