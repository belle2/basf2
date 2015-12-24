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
#include <algorithm>
#include <functional>

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

  bool operator<(const DAQLogMessage& lmsg, const DAQLogMessage& rmsg)
  {
    return lmsg.getDateInt() < rmsg.getDateInt();
  }

  bool operator>(const DAQLogMessage& lmsg, const DAQLogMessage& rmsg)
  {
    return lmsg.getDateInt() > rmsg.getDateInt();
  }

}

int main(int argc, const char** argv)
{
  if (argc < 2) {
    LogFile::debug("usage : %s <config> [-d]", argv[0]);
  }
  ConfigFile config("slowcontrol", argv[1]);
  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      daemon(0, 0);
    }
  }
  const std::string hostname = config.get("nsm.host");
  const int port = config.getInt("nsm.port");
  std::string nodename = config.get("nsm.nodename");
  NSMNode lognode(config.get("log.collector"));
  Log2NSMCallback* callback = new Log2NSMCallback(NSMNode(nodename));
  NSMNode& node(callback->getNode());
  NSMCommunicator* com = new NSMCommunicator();
  com->init(node, hostname, port);
  com->setCallback(callback);
  callback->init(*com);

  Inotify inotify;
  inotify.open();
  const std::string dirbase = config.get("dir.base");
  std::vector<std::string> dirs = StringUtil::split(config.get("dir.dir"), ',');
  std::map<std::string, unsigned long int> seek_m;
  /*
  while (true) {
    std::vector<DAQLogMessage> msgs;
    for (std::vector<std::string>::iterator it = dirs.begin();
   it != dirs.end(); it++) {
      std::string dir = dirbase + "/" + *it;
      DIR* dp = opendir(dir.c_str());
      struct dirent* dirst;
      while ((dirst = readdir(dp)) != NULL) {
  std::string filename = dirst->d_name;
  if (filename.find(".log") != std::string::npos) {
    if (seek_m.find(filename) == seek_m.end()) {
      struct stat st;
      std::string filepath = dir + "/" + filename;
      stat(filepath.c_str(), &st);
      LogFile::debug(filepath);
      seek_m.insert(std::map<std::string, unsigned long int>::value_type(filename, st.st_size));
    } else {
      std::string filepath = dir + "/" + filename;
      std::ifstream fin(filepath.c_str());
      fin.seekg(seek_m[filename]);
      //LogFile::debug("%s %d", filepath.c_str(), seek_m[filename]);
      struct stat st;
      stat(filepath.c_str(), &st);
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
    DAQLogMessage msg(StringUtil::toupper(*it), pri, message, t);
    msg.setId(com->getNodeIdByName(*it));
    msgs.push_back(msg);
        }
      }
    }
  }
      }
      closedir(dp);
    }
    std::sort(msgs.begin(), msgs.end(), std::less<DAQLogMessage>());
    for (size_t i = 0; i < msgs.size(); i++) {
      try {
  NSMCommunicator::send(NSMMessage(lognode, msgs[i], NSMCommand::LOGSET));
  LogFile::put(msgs[i].getPriority(), "[%s] [%s] [%s]", msgs[i].getDate().toString(),
         msgs[i].getNodeName().c_str(), msgs[i].getMessage().c_str());
  usleep(1000);
      } catch (const NSMHandlerException& e) {
  LogFile::error(e.what());
      }
    }
    sleep(1);
  }
  */
  std::vector<int> wds;
  for (std::vector<std::string>::iterator it = dirs.begin();
       it != dirs.end(); it++) {
    std::string dir = dirbase + "/" + *it;
    LogFile::debug(dir);
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
    InotifyEventList list(inotify.wait(1));
    std::vector<DAQLogMessage> msgs;
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
      } else if (it->getMask() == Inotify::FILE_OPEN) {
        struct stat st;
        stat(filename.c_str(), &st);
        seek_m.insert(std::map<std::string, unsigned long int>::value_type(filename, st.st_size));
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
            DAQLogMessage msg(StringUtil::toupper(dir), pri, message, t);
            msg.setId(com->getNodeIdByName(dir));
            msgs.push_back(msg);
          }
        }
      }
    }
    std::sort(msgs.begin(), msgs.end(), std::less<DAQLogMessage>());
    for (size_t i = 0; i < msgs.size(); i++) {
      try {
        NSMCommunicator::send(NSMMessage(lognode, msgs[i], NSMCommand::LOGSET));
        LogFile::put(msgs[i].getPriority(), "[%s] [%s] [%s]", msgs[i].getDate().toString(), msgs[i].getNodeName().c_str(),
                     msgs[i].getMessage().c_str());
        usleep(1000);
      } catch (const NSMHandlerException& e) {
        LogFile::error(e.what());
      }
    }
  }
  //*/
  return 0;
}
