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

#include <daq/slc/psql/PostgreSQLInterface.h>

#include <daq/slc/readout/RunInfoBuffer.h>

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/Time.h>
#include <daq/slc/system/PThread.h>
#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/Cond.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

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
  if (argc < 4) {
    printf("%s : hostname runtype user destination [expno runno]\n", argv[0]);
    return 1;
  }
  const char* hostname = argv[1];
  const char* runtype = argv[2];
  const char* user = argv[3];
  const char* destination = argv[4];
  const int expno = argc > 5 ? atoi(argv[5]) : 0;
  const int runno = argc > 6 ? atoi(argv[6]) : 0;
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  char sql[1000], s_expno[100], s_runno[100];
  sprintf(s_expno, ((expno > 0) ? "and expno = %d" : ""), expno);
  sprintf(s_runno, ((runno > 0) ? "and runno = %d" : ""), runno);
  sprintf(sql, "select id, path_daq from %s where time_copy is null and "
          "time_close is not null and path_daq like'%s:_%s' and "
          "runtype = '%s' %s %s order by id;",
          g_table, hostname, "%%", runtype, s_expno, s_runno);
  try {
    db.connect();
    db.execute(sql);
    DBRecordList record_v(db.loadRecords());
    db.close();
    char cmd[1000];
    for (size_t i = 0; i < record_v.size(); i++) {
      DBRecord& record(record_v[i]);
      int id = record.getInt("id");
      std::string path = record.get("path_daq");
      sprintf(cmd, "scp %s %s@%s", path.c_str(), user, destination);
      LogFile::info(cmd);
      system(cmd);
      StringList str = StringUtil::split(path, '/');
      std::string filename = str[str.size() - 1];
      sprintf(cmd, "update %s set time_copy = "
              "current_timestamp, path_cc = '%s:%s' where id = %d", g_table, destination, filename.c_str(), id);
      try {
        db.connect();
        db.execute(cmd);
        db.close();
      } catch (const DBHandlerException& e) {
        LogFile::error("Failed to access db for update: %s", e.what());
        return -1;
      }
    }
  } catch (const DBHandlerException& e) {
    LogFile::error("Failed to access db for read: %s", e.what());
    return -1;
  }

  return 0;
}

