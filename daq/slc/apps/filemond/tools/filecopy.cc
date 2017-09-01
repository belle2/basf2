#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/DAQLogMessage.h>

#include <daq/slc/system/Daemon.h>
#include <daq/slc/system/Inotify.h>
#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/File.h>

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

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    const std::string table = config.get("table");
    const std::string list_send = config.get("list.send");
    const std::string list_sent = config.get("list.sent");
    const std::string host = config.get("host");
    const std::string dest = config.get("dest");
    const std::vector<std::string> dirs = StringUtil::split(config.get("dirs"), ',');

    PostgreSQLInterface db(config.get("database.host"),
                           config.get("database.dbname"),
                           config.get("database.user"),
                           config.get("database.password"),
                           config.getInt("database.port"));
    db.connect();
    while (true) {
      db.execute("select * from %s where host = '%s' and time_close is not null "
                 "and time_sent is null order by time_close;", table.c_str(), host.c_str());
      DBRecordList records(db.loadRecords());
      if (!File::exist(list_sent)) {
        for (size_t i = 0; i < records.size(); i++) {
          std::string dir = records[i].get("dir");
          std::string name = records[i].get("name");
          std::string path = dir + "/" + name;
          if (File::exist(path)) {
            std::stringstream ss;
            ss << "scp " << path << " " << dest;
            std::string cmd = ss.str();
            LogFile::info(cmd);
            system(cmd.c_str());
            std::string d = Date().toString();
            db.execute("update %s set time_sent = '%s' where name = '%s' and host = '%s' time_sent is null;",
                       table.c_str(), d.c_str(), name.c_str(), host.c_str());
          }
        }
        std::ofstream fout(list_sent.c_str());
        for (size_t i = 0; i < records.size(); i++) {
          std::string name = records[i].get("name");
          fout << name << std::endl;
        }
      }
      sleep(30);
    }
  }

  return 0;
}
