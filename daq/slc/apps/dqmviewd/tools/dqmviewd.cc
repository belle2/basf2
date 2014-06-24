#include "daq/slc/apps/dqmviewd/DQMViewCallback.h"
#include "daq/slc/apps/dqmviewd/HistSender.h"

#include <daq/slc/apps/SocketAcceptor.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <cstring>
#include <cstdlib>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>

using namespace Belle2;

typedef SocketAcceptor<HistSender, DQMViewMaster> DQMUIAcceptor;

typedef void* MonitorFunc_t(const char*, const char*);

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("Usage: %s <nodename> ", argv[0]);
    return 1;
  }
  const char* name = argv[1];
  bool debugmode = (argc > 2);
  LogFile::open("dqmviewd");
  if (!debugmode) {
    system("killall hserver");
  }

  ConfigFile config("dqm");
  DQMViewMaster master;
  const std::string nodename = name;
  const std::string map_path = config.get("dqm.tmap.dir");
  const std::string hostname = config.get("dqm.host");
  const int port = config.getInt("dqm.port");

  StringList dqmlist = StringUtil::split(config.get("dqm.tmap.list"), ',');
  for (size_t i = 0; i < dqmlist.size(); i++) {
    StringList str = StringUtil::split(dqmlist[i], '/');
    const std::string pack_name = str[0];
    const std::string map_name = str[1];
    const int port = atoi(str[2].c_str());
    std::string emsg = StringUtil::form("Added DQM for %s", pack_name.c_str());
    std::string mapfile = map_path + "/" + map_name;
    ::unlink(mapfile.c_str());
    master.add(pack_name, port, mapfile);
  }
  PThread(new DQMUIAcceptor(hostname, port, master));
  LogFile::debug("Start socket acception from GUIs");

  NSMNode node(name);
  DQMViewCallback* callback = new DQMViewCallback(node, master);
  if (debugmode) {
    callback->init();
    while (true) { sleep(10); }
  } else {
    NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
    daemon->run();
  }
  return 0;
}

