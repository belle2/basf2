#include "daq/slc/apps/dqmviewd/DQMViewMaster.h"
#include "daq/slc/apps/dqmviewd/SimpleDQMPackage.h"

#include <daq/slc/apps/PackageSender.h>
#include <daq/slc/apps/SocketAcceptor.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/DynamicLoader.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/ConfigFile.h>

#include <cstdlib>
#include <dirent.h>

using namespace Belle2;

typedef SocketAcceptor<PackageSender, DQMViewMaster> DQMUIAcceptor;

typedef void* MonitorFunc_t(const char*, const char*, const char*);

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage: ./dqmserver <name> [<config>]");
    return 1;
  }
  //const char* name = argv[1];
  ConfigFile config((argc > 2) ? argv[2] : "dqm");

  std::vector<DynamicLoader*> dl_v;
  DQMViewMaster* master = new DQMViewMaster();
  const std::string lib_path = config.get("DQM_LIB_PATH");
  const std::string map_path = config.get("DQM_MAP_PATH");
  const std::string config_path = config.get("DQM_CONFIG_PATH");
  master->setDirectory(map_path);
  DIR* dir = opendir(map_path.c_str());
  if (dir != NULL) {
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
      if (entry->d_type == DT_REG) {
        std::string filename = entry->d_name;
        if (filename.find(".conf") != std::string::npos) {
          Belle2::debug("%s:%d %s", __FILE__, __LINE__, filename.c_str());
          config.clear();
          config.read(map_path + "/" + filename);
          std::string pack_name = config.get("DQM_PACKAGE_NAME");
          if (pack_name.size() == 0) continue;
          std::string pack_title = config.get("DQM_PACKAGE_TITLE");
          if (pack_title.size() == 0) pack_title = pack_name;
          std::string pack_map   = config.get("DQM_PACKAGE_MAP");
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
              (DQMPackage*)createMonitor(pack_name.c_str(), pack_title.c_str(),
                                         pack_map.c_str());
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
  Belle2::PThread(new DQMUIAcceptor(config.get("DQM_GUI_HOST"),
                                    config.getInt("DQM_GUI_PORT"), master));
  master->run();
  return 0;
}

