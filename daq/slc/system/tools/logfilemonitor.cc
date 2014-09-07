#include <daq/slc/database/PostgreSQLInterface.h>
#include <daq/slc/database/DAQLogMessage.h>
#include <daq/slc/database/LoggerObjectTable.h>

#include <daq/slc/system/Inotify.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/Date.h>
#include <daq/slc/base/ConfigFile.h>

#include <map>
#include <fstream>
#include <iostream>
#include <cstdio>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

using namespace Belle2;

time_t toUnixtime(const char* str)
{
  struct tm t;
  sscanf(str, "%d:%d:%d",
         &t.tm_hour, &t.tm_min, &t.tm_sec);
  Date d;
  t.tm_year =  d.getYear() - 1900;
  t.tm_mon = d.getMonth() - 1;
  t.tm_mday = d.getDay();
  return mktime(&t);
}

int main()
{
  daemon(0, 0);
  Inotify inotify;
  inotify.open();
  std::vector<std::string> dirs;
  dirs.push_back("collector");
  dirs.push_back("distributor");
  dirs.push_back("dqmserver");
  dirs.push_back("evp_hltwk01");
  dirs.push_back("evp_hltwk02");
  dirs.push_back("evp_hltwk03");
  dirs.push_back("evp_hltwk04");
  dirs.push_back("evp_hltwk05");
  dirs.push_back("evp_hltwk06");
  dirs.push_back("evp_hltwk07");
  dirs.push_back("evp_hltwk08");
  dirs.push_back("evp_hltwk09");
  dirs.push_back("evp_hltwk10");
  dirs.push_back("evp_hltwk11");
  dirs.push_back("evp_hltwk12");
  dirs.push_back("evp_hltwk13");
  dirs.push_back("evp_hltwk14");
  dirs.push_back("evp_hltwk15");
  dirs.push_back("evp_hltwk16");
  dirs.push_back("evp_hltwk17");
  dirs.push_back("evp_hltwk18");
  dirs.push_back("evp_hltwk19");
  dirs.push_back("evp_hltwk20");
  dirs.push_back("roisender");
  std::map<std::string, unsigned long int> seek_m;
  std::vector<int> wds;
  for (std::vector<std::string>::iterator it = dirs.begin();
       it != dirs.end(); it++) {
    std::string dir = "/home/usr/hltdaq/run/" + *it;
    wds.push_back(inotify.add(dir,
                              Inotify::FILE_CREATE |
                              Inotify::FILE_OPEN |
                              Inotify::FILE_CLOSE_WRITE |
                              Inotify::FILE_MODIFY));
    DIR* dp = opendir(dir.c_str());
    struct dirent* dirst;
    while ((dirst = readdir(dp)) != NULL) {
      std::string filename = dirst->d_name;
      if (filename.find(".log") != std::string::npos) {
        struct stat st;
        filename = dir + "/" + filename;
        stat(filename.c_str(), &st);
        seek_m.insert(std::map<std::string, unsigned long int>::value_type(filename, st.st_size));
      }
    }
    closedir(dp);
  }
  ConfigFile config("slowcontrol");
  DBInterface* db = new PostgreSQLInterface(config.get("database.host"),
                                            config.get("database.dbname"),
                                            config.get("database.user"),
                                            config.get("database.password"),
                                            config.getInt("database.port"));
  while (true) {
    InotifyEventList list(inotify.wait(10));
    if (log.getPriority() > LogFile::DEBUG) {
      for (InotifyEventList::iterator it = list.begin();
           it != list.end(); it++) {
        int wd = it->get_wd();
        std::string filename = it->getName();
        if (filename.find(".log") == std::string::npos) continue;
        std::string dir;
        for (size_t i = 0; i < wds.size(); i++) {
          if (wd == wds[i]) {
            dir = dirs[i];
            filename = "/home/usr/hltdaq/run/" + dir + "/" + filename;
            break;
          }
        }
        if (it->getMask() == Inotify::FILE_CREATE) {
          struct stat st;
          stat(filename.c_str(), &st);
          seek_m.insert(std::map<std::string, unsigned long int>::value_type(filename, st.st_size));
          //LogFile::debug("file %s created", it->getName().c_str());
        } else if (it->getMask() == Inotify::FILE_OPEN) {
          struct stat st;
          stat(filename.c_str(), &st);
          seek_m.insert(std::map<std::string, unsigned long int>::value_type(filename, st.st_size));
          //LogFile::debug("file %s opened", it->getName().c_str());
        } else if (it->getMask() == Inotify::FILE_CLOSE_WRITE) {
          //LogFile::debug("file %s closed", it->getName().c_str());
        } else if (it->getMask() == Inotify::FILE_MODIFY) {
          //LogFile::debug("file %s (%d) modified", filename.c_str(), seek_m[filename]);
          std::ifstream fin(filename.c_str());
          fin.seekg(seek_m[filename]);
          std::string buf;
          while (fin && getline(fin, buf)) {
            unsigned int s = buf.find(" ");
            std::string timestamp = buf.substr(0, s);
            std::string message = buf.substr(s + 1);
            time_t t = toUnixtime(timestamp.c_str());
            DAQLogMessage log(dir, LogFile::DEBUG, message, t);
            log.setNode("HLT");
            if (message.find("[INFO] ") == 0) {
              LogFile::info(message);
              //log.setPriority(LogFile::INFO);
            } else if (message.find("[NOTICE] ") == 0) {
              LogFile::notice(message);
              //log.setPriority(LogFile::NOTICE);
            } else if (message.find("[WARNING] ") == 0) {
              LogFile::warning(message);
              //log.setPriority(LogFile::WARNING);
            } else if (message.find("[ERROR] ") == 0) {
              LogFile::error(message);
              //log.setPriority(LogFile::ERROR);
            } else if (message.find("[FATAL] ") == 0) {
              LogFile::fatal(message);
              //log.setPriority(LogFile::FATAL);
            } else {
              //LogFile::debug(message);
            }
            try {
              db->connect();
              LoggerObjectTable(db).add(log, true);
            } catch (const DBHandlerException& e) {

            }
            db->close();
          }
        }
      }
    }
  }
  return 0;
}
