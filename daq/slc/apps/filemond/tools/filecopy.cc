#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/DAQLogMessage.h>

#include <daq/slc/system/Inotify.h>
#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/File.h>
#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/Date.h>
#include <daq/slc/base/ConfigFile.h>

#include <map>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <algorithm>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    const std::string table = config.get("table");
    const std::string list_send = config.get("list.send");
    const std::string list_sent = config.get("list.sent");
    const std::string host = config.get("host");
    const std::vector<std::string> dirs = StringUtil::split(config.get("dirs"), ',');

    PostgreSQLInterface db(config.get("database.host"),
                           config.get("database.dbname"),
                           config.get("database.user"),
                           config.get("database.password"),
                           config.getInt("database.port"));

    db.connect();
    db.execute("select * from %s where time_sent is null and host = '%s' order by time_close;",
               table.c_str(), host.c_str());
    DBRecordList records(db.loadRecords());
    if (!File::exist(list_send.c_str())) {
      for (size_t i = 0; i < records.size(); i++) {
        std::string path = records[i].get("path");
        std::string name = records[i].get("name");
        std::string label = records[i].get("label");
        int id = records[i].getInt("id");
        int expno = records[i].getInt("expno");
        int runno = records[i].getInt("runno");
        int fileno = records[i].getInt("fileno");
        std::string size = records[i].get("size");
        std::string nevents = records[i].get("nevents");
        std::string chksum = records[i].get("chksum");
        std::string opt = (fileno > 0) ? StringUtil::form("-%d", fileno) : "";
        /*
        std::string cmd = StringUtil::form("mkdir -p /rawdata/disk01/storage/%4.4d/%5.5d", expno, runno);
        LogFile::info(cmd);
        system(cmd.c_str());
        cmd = StringUtil::form("cp -p %s /rawdata/disk01/storage/%4.4d/%5.5d/%s.%4.4d.%5.5d.sroot%s",
                   path.c_str(), expno, runno, label.c_str(), expno, runno, opt.c_str());
        LogFile::info(cmd);
        system(cmd.c_str());
        */
        db.execute("update %s set path='/rawdata/disk01/storage/%4.4d/%5.5d/%s.%4.4d.%5.5d.sroot%s', name='%s.%4.4d.%5.5d.sroot%s' where id=%d",
                   table.c_str(), expno, runno, label.c_str(), expno, runno, opt.c_str(), label.c_str(), expno, runno, opt.c_str(), id);
      }
    }
  }

  return 0;
}

