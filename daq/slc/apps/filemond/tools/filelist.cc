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
    while (true) {
      db.execute("select * from %s where time_sent is null and host = '%s' order by time_close;",
                 table.c_str(), host.c_str());
      DBRecordList records(db.loadRecords());
      if (!File::exist(list_send.c_str())) {
        std::ofstream fout(list_send.c_str());
        for (size_t i = 0; i < records.size(); i++) {
          std::string path = records[i].get("dir") + "/" + records[i].get("name");
          LogFile::debug(path);
          fout << path << std::endl;
        }
      }
      sleep(30);
      if (File::exist(list_send.c_str())) {
        std::ifstream fin(list_sent.c_str());
        std::string name;
        std::string d = Date().toString();
        while (fin >> name) {
          try {
            db.execute("update %s set time_sent = '%s' where name = '%s' and host = '%s' time_sent is null;",
                       table.c_str(), d.c_str(), host.c_str(), name.c_str());
          } catch (const DBHandlerException& e) {

          }
        }
        fin.close();
        unlink(list_sent.c_str());
      }
      sleep(30);
    }
  }

  return 0;
}
