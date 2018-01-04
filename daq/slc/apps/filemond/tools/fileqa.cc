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
#include <sstream>
#include <cstdio>
#include <algorithm>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <stdio.h>
#include <stdint.h>
#include "zlib.h"

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    const std::string m_table = config.get("table");
    const std::string m_homedir = config.get("homedir");
    const std::vector<std::string> m_dirs = StringUtil::split(config.get("dirs"), ',');

    PostgreSQLInterface db(config.get("database.host"),
                           config.get("database.dbname"),
                           config.get("database.user"),
                           config.get("database.password"),
                           config.getInt("database.port"));
    while (true) {
      try {
        db.connect();
        //db.execute("select path,host,fileno from datafiles where time_close is not null order by id desc;");
        db.execute("select path,host,label,fileno,runno,expno from datafiles where label != 'null' and "
                   "time_close is not null and time_close > (current_timestamp - cast('14 day' as interval)) order by id desc;");
        //, m_table.c_str());
        DBRecordList records(db.loadRecords());
        LogFile::info("Done ");
        if (records.size() > 0) {
          for (size_t i = 0; i < records.size(); i++) {
            int fileno = records[i].getInt("fileno");
            std::string label = records[i].get("label");
            if (fileno == 0 || (label != "debug" && label != "cosmic" &&
                                label != "beam" && label != "test")) {
              int expno = records[i].getInt("expno");
              int runno = records[i].getInt("runno");
              std::string path = records[i].get("path");
              std::string host = records[i].get("host");
              host = StringUtil::replace(host, "HLT10", "store10");
              host = StringUtil::replace(host, "HLT", "store0");
              host += ".daqnet.kek.jp";
              std::stringstream ss;
              ss << "/data" << (runno % 4 + 1) << "/storage/srootfiles/" << StringUtil::form("%04d/%05d/", expno, runno);
              std::string dir = ss.str();
              ss.str("");
              ss << "mkdir -p " << dir;
              std::string exe = ss.str();
              ::system(exe.c_str());
              LogFile::info(exe);
              ss.str("");
              ss << "rsync -e ssh -av " << host << ":" << path << " " << dir;
              exe = ss.str();
              ::system(exe.c_str());
              LogFile::info(exe);
            }
          }
        }
      } catch (const DBHandlerException& e) {
        LogFile::fatal(e.what());
        return 1;
      }
      sleep(60);
    }
  }

  return 0;
}
