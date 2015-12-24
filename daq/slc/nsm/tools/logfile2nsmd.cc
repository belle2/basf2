#include <daq/slc/nsm/NSMCallback.h>
#include <daq/slc/nsm/NSMCommunicator.h>
#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/Inotify.h>
#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

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

namespace Belle2 {

  class Log2NSMCallback : public NSMCallback {

  public:
    Log2NSMCallback(const NSMNode& node)
      : NSMCallback(5)
    {
      setNode(node);
    }
    virtual ~Log2NSMCallback() throw() {}

  public:
    virtual void init(NSMCommunicator&) throw()
    {
      try {
      } catch (const IOException& e) {
      }
    }

  };

}

int main(int argc, const char** argv)
{
  if (argc < 2) {
    LogFile::debug("usage : %s <config>", argv[0]);
  }
  ConfigFile config("slowcontrol", argv[1]);
  const std::string hostname = config.get("nsm.host");
  const int port = config.getInt("nsm.port");
  std::string nodename = config.get("nsm.nodename");
  NSMNode lognode(config.get("log.collector"));
  Log2NSMCallback* callback = new Log2NSMCallback(NSMNode(nodename));
  PThread(new NSMNodeDaemon(callback, hostname, port));

  //daemon(0, 0);
  Inotify inotify;
  inotify.open();
  const std::string dirbase = config.get("dir.base");
  std::vector<std::string> dirs = StringUtil::split(config.get("dir.dir"), ',');
  std::map<std::string, unsigned long int> seek_m;
  std::vector<int> wds;
  for (std::vector<std::string>::iterator it = dirs.begin();
       it != dirs.end(); it++) {
    std::string dir = dirbase + *it;
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

  while (true) {
    InotifyEventList list(inotify.wait(10));
    for (InotifyEventList::iterator it = list.begin();
         it != list.end(); it++) {
      int wd = it->get_wd();
      std::string filename = it->getName();
      if (filename.find(".log") == std::string::npos) continue;
      std::string dir;
      for (size_t i = 0; i < wds.size(); i++) {
        if (wd == wds[i]) {
          dir = dirs[i];
          filename = dirbase + "/" + dir + "/" + filename;
          break;
        }
      }
      if (it->getMask() == Inotify::FILE_CREATE) {
        struct stat st;
        stat(filename.c_str(), &st);
        seek_m.insert(std::map<std::string, unsigned long int>::value_type(filename, st.st_size));
        LogFile::debug("file %s created", it->getName().c_str());
      } else if (it->getMask() == Inotify::FILE_OPEN) {
        struct stat st;
        stat(filename.c_str(), &st);
        seek_m.insert(std::map<std::string, unsigned long int>::value_type(filename, st.st_size));
        LogFile::debug("file %s opened", it->getName().c_str());
      } else if (it->getMask() == Inotify::FILE_CLOSE_WRITE) {
        //LogFile::debug("file %s closed", it->getName().c_str());
      } else if (it->getMask() == Inotify::FILE_MODIFY) {
        //LogFile::debug("file %s (%d) modified", filename.c_str(), seek_m[filename]);
        std::ifstream fin(filename.c_str());
        fin.seekg(seek_m[filename]);
        struct stat st;
        stat(filename.c_str(), &st);
        seek_m[filename] = st.st_size;
        std::string buf;
        while (fin && getline(fin, buf)) {
          unsigned int s = buf.find(" ");
          std::string timestamp = buf.substr(0, s);
          std::string message = buf.substr(s + 1);
          time_t t = toUnixtime(timestamp.c_str());
          LogFile::Priority pri = LogFile::DEBUG;
          if (message.find("[INFO] ") == 0) {
            message = message.substr(7);
            pri = LogFile::INFO;
          } else if (message.find("[NOTICE] ") == 0) {
            message = message.substr(9);
            pri = LogFile::NOTICE;
          } else if (message.find("[WARNING] ") == 0) {
            message = message.substr(10);
            pri = LogFile::WARNING;
          } else if (message.find("[ERROR] ") == 0) {
            message = message.substr(8);
            pri = LogFile::ERROR;
          } else if (message.find("[FATAL] ") == 0) {
            message = message.substr(8);
            pri = LogFile::FATAL;
          } else {
          }
          if (pri > LogFile::DEBUG) {
            DAQLogMessage msg(dir, pri, message, t);
            LogFile::put(pri, message);
            try {
              NSMCommunicator::send(NSMMessage(lognode, msg));
            } catch (const NSMHandlerException& e) {
              LogFile::error(e.what());
            }
          }
        }
      }
    }
  }
  return 0;
}
