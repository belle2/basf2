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

typedef std::pair<std::string, std::string> fileinfo_t;
bool sort_less(const fileinfo_t& left, const fileinfo_t& right)
{
  return left.second < right.second;
}

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
    db.execute("select * from %s where time_sent is not null order by time_close;",
               table.c_str());
    DBRecordList records(db.loadRecords());
    std::vector<fileinfo_t> infos;
    std::map<std::string, std::string> names;
    for (size_t i = 0; i < records.size(); i++) {
      infos.push_back(fileinfo_t(records[i].get("name"), records[1].get("time_close")));
      names.insert(fileinfo_t(records[i].get("name"), records[1].get("time_close")));
    }

    Inotify inotify;
    inotify.open();
    std::vector<int> wds;
    time_t t0;
    time(&t0);
    for (std::vector<std::string>::const_iterator it = dirs.begin();
         it != dirs.end(); it++) {
      std::string dir = *it;
      wds.push_back(inotify.add(dir, Inotify::FILE_CLOSE_WRITE));
      DIR* dp = opendir(dir.c_str());
      struct dirent* dirst;
      while ((dirst = readdir(dp)) != NULL) {
        std::string name = dirst->d_name;
        if (name.find(".sroot") != std::string::npos && name.find(".gz") == std::string::npos) {
          std::string filepath = dir + "/" + name;
          if (names.find(name) == names.end()) {
            struct stat st;
            stat(filepath.c_str(), &st);
            if ((t0 - st.st_mtime) > 60 * 3) {
              std::string d = Date(st.st_mtime).toString();
              try {
                db.execute("insert into %s (name, dir, host, time_close) "
                           "values ('%s', '%s', '%s', '%s');",
                           table.c_str(), name.c_str(), dir.c_str(), host.c_str(), d.c_str());
                LogFile::info("new file: %s (%s)", name.c_str(), d.c_str());
              } catch (const DBHandlerException& e) {

              }
            }
          }
        }
      }
      closedir(dp);
    }

    while (true) {
      InotifyEventList list(inotify.wait(10));
      for (InotifyEventList::iterator it = list.begin();
           it != list.end(); it++) {
        int wd = it->get_wd();
        std::string name = it->getName();
        if (name.find(".sroot") != std::string::npos && name.find(".gz") == std::string::npos) {
          std::string dir;
          for (size_t i = 0; i < wds.size(); i++) {
            if (wd == wds[i] && it->getMask() == Inotify::FILE_CLOSE_WRITE) {
              dir = dirs[i];
              std::string filepath = dir + name;
              struct stat st;
              stat(filepath.c_str(), &st);
              std::string d = Date(st.st_mtime).toString();
              db.execute("insert into %s (name, dir, host, time_close) "
                         "values ('%s', '%s', '%s', '%s');",
                         table.c_str(), name.c_str(), dir.c_str(), host.c_str(), d.c_str());
              LogFile::info("new file: %s (%s)", name.c_str(), d.c_str());
              break;
            }
          }
        }
      }
    }
  }

  return 0;
}
