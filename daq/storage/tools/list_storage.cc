//+
// File : rawfile2rb.cc
// Description : Read raw data dump file and put record in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <unistd.h>
#include <cstdlib>

#include <framework/logging/Logger.h>
#include <framework/pcore/SeqFile.h>

#include <daq/storage/BinData.h>
#include <daq/storage/SharedEventBuffer.h>

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/readout/RunInfoBuffer.h>

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/Time.h>
#include <daq/slc/system/PThread.h>
#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/Cond.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <queue>

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
    printf("%s : [-all] [-h hostname] [-t runtype] [-e expnumber] [-r runnumber] [-n number_of_file ] \n", argv[0]);
    return 1;
  }
  std::string hostname;
  std::string runtype;
  int expno = 0;
  int runno = 0;
  int max = 0;
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
    }
  }
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  std::stringstream ss;
  ss << "select path_daq from " << g_table << " "
     << "where time_copy is null and time_close is not null ";
  if (hostname.size() > 0) ss << "and path_daq like '" << hostname << ":_%%' ";
  if (runtype.size() > 0) ss << "and runtype = '" << runtype << "' ";
  if (expno > 0) ss << "and expno = " << expno << " ";
  if (runno > 0) ss << "and runno = " << runno << " ";
  ss << " order by id desc";
  if (max > 0) ss << " limit " << max << " ";
  try {
    db.connect();
    db.execute(ss.str().c_str());
    DBRecordList record_v(db.loadRecords());
    db.close();
    for (size_t i = 0; i < record_v.size(); i++) {
      DBRecord& record(record_v[i]);
      std::cout << record.get("path_daq") << std::endl;
    }
  } catch (const DBHandlerException& e) {
    LogFile::error("Failed to access db for read: %s", e.what());
    return -1;
  }

  return 0;
}

