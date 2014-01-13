#include "daq/slc/apps/envmonitord/EnvMonitorMaster.h"
#include "daq/slc/apps/envmonitord/EnvDBRecorder.h"

#include <daq/slc/apps/PackageSender.h>
#include <daq/slc/apps/SocketAcceptor.h>

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/DynamicLoader.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/ConfigFile.h>

#include <cstring>
#include <cstdlib>
#include <dirent.h>
#include <errno.h>

using namespace Belle2;

typedef SocketAcceptor<PackageSender, EnvMonitorMaster> EnvUIAcceptor;

typedef void* MonitorFunc_t(const char*);

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage : ./nsmmond <name>");
    return 1;
  }
  LogFile::open("envmonitord");
  const char* name = argv[1];
  NSMNode* node = new NSMNode(name);
  NSMCommunicator* comm = new NSMCommunicator(node);
  comm->init();

  ConfigFile config("slowcontrol", "envmon");
  DBInterface* db = new PostgreSQLInterface(config.get("DATABASE_HOST"),
                                            config.get("DATABASE_NAME"),
                                            config.get("DATABASE_USER"),
                                            config.get("DATABASE_PASS"),
                                            config.getInt("DATABASE_PORT"));
  EnvDBRecorder::setDB(db);
  const std::string lib_path = config.get("ENV_LIB_PATH");
  const std::string map_path = config.get("ENV_MAP_PATH");
  const std::string config_path = config.get("ENV_CONFIG_PATH");
  const std::string hostname = config.get("ENV_GUI_HOST");
  const int port = config.getInt("ENV_GUI_PORT");

  std::vector<DynamicLoader*> dl_v;
  EnvMonitorMaster* master = new EnvMonitorMaster(comm);
  DIR* dir = opendir(config_path.c_str());
  int count = 0;
  if (dir != NULL) {
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
      if (entry->d_type == DT_REG) {
        std::string filename = entry->d_name;
        if (filename.find(".conf") != std::string::npos &&
            filename.find("~") == std::string::npos) {
          config.clear();
          config.read(config_path + "/" + filename);
          std::string pack_name = config.get("ENV_PACKAGE_NAME");
          if (pack_name.size() == 0) continue;
          Belle2::debug("Env config (%d): %s", count++, filename.c_str());
          std::string pack_lib   = config.get("ENV_PACKAGE_LIB");
          std::string pack_class = config.get("ENV_PACKAGE_CLASS");
          std::string nsmdata_name   = config.get("ENV_NSMDATA_NAME");
          std::string nsmdata_format  = config.get("ENV_NSMDATA_FORMAT");
          const int nsmdata_revision  = config.getInt("ENV_NSMDATA_REVISION");
          if (pack_class.size() > 0) {
            if (pack_lib.size() > 0 && pack_lib.at(0) != '/') {
              pack_lib = "lib" + pack_lib + ".so";
            }
            DynamicLoader* dl = new DynamicLoader();
            dl->open(pack_lib);
            std::string funcname = Belle2::form("create%s", pack_class.c_str());
            MonitorFunc_t* createMonitor = (MonitorFunc_t*)dl->load(funcname);
            EnvMonitorPackage* package = (EnvMonitorPackage*)createMonitor(pack_name.c_str());
            if (nsmdata_name.size() > 0 && nsmdata_format.size() > 0 && nsmdata_revision > 0) {
              package->setData(new NSMData(nsmdata_name, nsmdata_format, nsmdata_revision));
            }
            dl_v.push_back(dl);
            master->add(package);
          }
        }
      }
    }
    closedir(dir);
  } else {
    std::string emsg = Belle2::form("Failed to find directory : %s", strerror(errno));
    LogFile::fatal(emsg.c_str());
    throw (Exception(__FILE__, __LINE__, emsg));
  }
  PThread(new EnvUIAcceptor(hostname, port, master));
  LogFile::debug("Start socket acception from GUIs");
  master->run();
  return 0;
}
