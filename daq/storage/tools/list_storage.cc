#include <daq/slc/psql/PostgreSQLInterface.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <sys/statvfs.h>

using namespace Belle2;

const char* g_table = "fileinfo";

int main(int argc, char** argv)
{
  if (argc < 2 || strcmp(argv[1], "-help") == 0) {
    printf("%s : [-all] [-h hostname] [-t runtype] [-e expnumber] [-r runnumber] [-n number_of_file ] [-f] [--copied]\n", argv[0]);
    return 1;
  }
  std::string hostname;
  std::string runtype;
  int expno = 0;
  int runno = 0;
  int max = 0;
  bool firstonly = false;
  bool copied = false;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      i++;
      if (i < argc) hostname = argv[i];
    } else if (strcmp(argv[i], "-t") == 0) {
      i++;
      if (i < argc) runtype = argv[i];
    } else if (strcmp(argv[i], "-e") == 0) {
      i++;
      if (i < argc) expno = atoi(argv[i]);
    } else if (strcmp(argv[i], "-r") == 0) {
      i++;
      if (i < argc) runno = atoi(argv[i]);
    } else if (strcmp(argv[i], "-n") == 0) {
      i++;
      if (i < argc) max = atoi(argv[i]);
    } else if (strcmp(argv[i], "-f") == 0) {
      firstonly = true;
    } else if (strcmp(argv[i], "--copied") == 0) {
      firstonly = true;
      copied = true;
    }
  }
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  std::stringstream ss;
  ss << "select path_sroot, path_root from " << g_table << " "
     << "where runno > 0 ";
  //   << "where time_copy is null and time_close is not null ";
  if (hostname.size() > 0) ss << "and path_sroot like '" << hostname << ":_%%' ";
  if (runtype.size() > 0) ss << "and runtype = '" << runtype << "' ";
  if (expno > 0) ss << "and expno = " << expno << " ";
  if (runno > 0) ss << "and runno = " << runno << " ";
  if (firstonly) ss << "and fileid = " << 0 << " ";
  if (copied) ss << "and time_covert is not null ";
  ss << " order by id desc";
  if (max > 0) ss << " limit " << max << " ";
  try {
    db.connect();
    db.execute(ss.str().c_str());
    DBRecordList record_v(db.loadRecords());
    db.close();
    for (size_t i = 0; i < record_v.size(); i++) {
      DBRecord& record(record_v[i]);
      if (copied)
        std::cout << record.get("path_root") << std::endl;
      else
        std::cout << record.get("path_sroot") << std::endl;
    }
  } catch (const DBHandlerException& e) {
    LogFile::error("Failed to access db for read: %s", e.what());
    return -1;
  }

  return 0;
}

