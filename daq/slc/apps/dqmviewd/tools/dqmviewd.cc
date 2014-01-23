#include "daq/slc/apps/dqmviewd/DQMViewCallback.h"
#include "daq/slc/apps/dqmviewd/HistSender.h"

#include <daq/slc/apps/SocketAcceptor.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Debugger.h>
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
  if (argc < 1) {
    Belle2::debug("Usage: ./dqmviewd");
    return 1;
  }
  daemon(0, 0);
  LogFile::open("dqmviewd");
  //system("killall hserver");
  const char* name = argv[1];

  ConfigFile config("dqm");
  DQMViewMaster* master = new DQMViewMaster();
  const std::string nodename = config.get("DQM_NSM_NAME");
  const std::string map_path = config.get("DQM_MAP_PATH");
  const std::string config_path = config.get("DQM_CONFIG_PATH");
  const std::string hostname = config.get("DQM_GUI_HOST");
  const int port = config.getInt("DQM_GUI_PORT");
  DIR* dir = opendir(config_path.c_str());
  if (dir != NULL) {
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
      if (entry->d_type == DT_REG) {
        std::string filename = entry->d_name;
        if (filename.find(".conf") != std::string::npos &&
            filename.find("~") == std::string::npos) {
          config.clear();
          config.read(config_path + "/" + filename);
          std::string pack_name = config.get("DQM_PACKAGE_NAME");
          int pack_port = config.getInt("DQM_PACKAGE_PORT");
          std::string pack_map   = config.get("DQM_PACKAGE_MAP");
          if (pack_name.size() == 0 || pack_port <= 0 || pack_map.size() == 0)
            continue;
          std::string emsg = Belle2::form("Added DQM for %s", pack_name.c_str());
          master->add(pack_name, pack_port, map_path + "/" + pack_map);
        }
      }
    }
    closedir(dir);
  } else {
    std::string emsg = Belle2::form("Failed to find directory : %s", strerror(errno));
    LogFile::fatal(emsg.c_str());
    throw (Exception(__FILE__, __LINE__, emsg));
  }
  PThread(new DQMUIAcceptor(hostname, port, master));
  LogFile::debug("Start socket acception from GUIs");

  NSMNode* node = new NSMNode(name);
  DQMViewCallback* callback = new DQMViewCallback(node, master);
  /*
  callback->init();
  while (true) { sleep(10); }
  */
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
  daemon->run();
  return 0;
}

