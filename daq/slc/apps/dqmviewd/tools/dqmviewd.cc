#include "daq/slc/apps/dqmviewd/DQMViewMaster.h"
#include "daq/slc/apps/dqmviewd/SimpleDQMPackage.h"

#include <daq/slc/apps/PackageSender.h>
#include <daq/slc/apps/SocketAcceptor.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/Fork.h>
#include <daq/slc/system/Executor.h>
#include <daq/slc/system/DynamicLoader.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/ConfigFile.h>

#include <cstdlib>
#include <dirent.h>
#include <errno.h>

namespace Belle2 {

  class HSeverExecutor {
  public:
    HSeverExecutor(int port, const std::string& mapfile, const std::string& dir)
      : _port(port), _mapfile(mapfile), _dir(dir) {}

  public:
    void run() {
      Executor executor;
      executor.setExecutable("hserver");
      executor.addArg(Belle2::form("%d", _port));
      executor.addArg(_dir + "/" + _mapfile);
      executor.execute();
    }

  private:
    int _port;
    std::string _mapfile;
    std::string _dir;

  };

}

using namespace Belle2;

typedef SocketAcceptor<PackageSender, DQMViewMaster> DQMUIAcceptor;

typedef void* MonitorFunc_t(const char*, const char*);

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage: ./dqmserver <name> [<config>]");
    return 1;
  }
  system("killall hserver");
  //const char* name = argv[1];
  ConfigFile config((argc > 2) ? argv[2] : "dqm");

  std::vector<DynamicLoader*> dl_v;
  DQMViewMaster* master = new DQMViewMaster();
  const std::string lib_path = config.get("DQM_LIB_PATH");
  const std::string map_path = config.get("DQM_MAP_PATH");
  const std::string config_path = config.get("DQM_CONFIG_PATH");
  const std::string hostname = config.get("DQM_GUI_HOST");
  const int port = config.getInt("DQM_GUI_PORT");
  master->setDirectory(map_path);
  DIR* dir = opendir(config_path.c_str());
  int count = 0;
  if (dir != NULL) {
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
      if (entry->d_type == DT_REG) {
        std::string filename = entry->d_name;
        if (filename.find(".conf") != std::string::npos) {
          config.clear();
          config.read(config_path + "/" + filename);
          std::string pack_name = config.get("DQM_PACKAGE_NAME");
          int pack_port = config.getInt("DQM_PACKAGE_PORT");
          if (pack_name.size() == 0 || pack_port == 0) continue;
          std::string pack_map   = config.get("DQM_PACKAGE_MAP");
          Belle2::debug("DQM config (%d): %s", count++, filename.c_str());
          Belle2::debug("DQM name = %s, port = %d, map file = %s",
                        pack_name.c_str(), pack_port, pack_map.c_str());
          Belle2::debug("booting hserver with port = %d mapfile = %s/%s",
                        pack_port, map_path.c_str(), pack_map.c_str());
          Fork(new HSeverExecutor(pack_port, pack_map, map_path));
          std::string pack_lib   = config.get("DQM_PACKAGE_LIB");
          std::string pack_class = config.get("DQM_PACKAGE_CLASS");
          if (pack_class.size() > 0) {
            if (pack_lib.size() > 0 && pack_lib.at(0) != '/') {
              pack_lib = "lib" + pack_lib + ".so";
            }
            DynamicLoader* dl = new DynamicLoader();
            dl->open(pack_lib);
            MonitorFunc_t* createMonitor =
              (MonitorFunc_t*)dl->load(Belle2::form("create%s", pack_class.c_str()));
            DQMPackage* package =
              (DQMPackage*)createMonitor(pack_name.c_str(), pack_map.c_str());
            dl_v.push_back(dl);
            master->add(package);
          } else {
            master->add(new SimpleDQMPackage(pack_name, pack_map));
          }
        }
      }
    }
    closedir(dir);
  } else {
    throw (Exception(__FILE__, __LINE__,
                     Belle2::form("Failed to find directory : %s", strerror(errno))));
  }
  Belle2::PThread(new DQMUIAcceptor(hostname, port, master));
  Belle2::debug("Start socket acception from GUIs");
  master->run();
  return 0;
}

