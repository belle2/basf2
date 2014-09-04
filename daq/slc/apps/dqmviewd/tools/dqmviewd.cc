#include "daq/slc/apps/dqmviewd/DQMViewCallback.h"
#include "daq/slc/apps/dqmviewd/HistSender.h"

#include <daq/slc/apps/SocketAcceptor.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/Daemon.h>
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
  ConfigFile config("slowcontrol", "dqm");
  const std::string nodename = config.get("nsm.nodename");
  if (!Daemon::start(("dqmviewd." + nodename).c_str(), argc, argv)) {
    return 1;
  }
  DQMViewMaster master;
  const std::string map_path = config.get("dqm.tmap.dir");
  const std::string hostname = config.get("dqm.host");
  const int port = config.getInt("dqm.port");

  StringList dqmlist = StringUtil::split(config.get("dqm.tmap.list"), ',');
  for (size_t i = 0; i < dqmlist.size(); i++) {
    const std::string pack_name = config.get(StringUtil::form("dqm.%s.name", dqmlist[i].c_str()));
    const std::string map_name = config.get(StringUtil::form("dqm.%s.file", dqmlist[i].c_str()));
    const int port = config.getInt(StringUtil::form("dqm.%s.port", dqmlist[i].c_str()));
    std::string mapfile = map_path + "/" + map_name;
    master.add(pack_name, port, mapfile);
  }
  PThread(new DQMUIAcceptor(hostname, port, master));
  LogFile::debug("Start socket acception from GUIs");

  DQMViewCallback* callback = new DQMViewCallback(NSMNode(nodename), master);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback, config.get("nsm.global.host"),
                                            config.getInt("nsm.global.port"));
  daemon->run();
  return 0;
}

