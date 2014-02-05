#include "daq/slc/apps/envmonitord/EnvMonitorMaster.h"
#include "daq/slc/apps/envmonitord/DataSender.h"

#include <daq/slc/apps/SocketAcceptor.h>

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/DynamicLoader.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/ConfigFile.h>

#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

using namespace Belle2;

typedef SocketAcceptor<DataSender, EnvMonitorMaster> EnvUIAcceptor;

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
  const std::string config_path = config.get("ENV_CONFIG_PATH");
  const std::string hostname = config.get("ENV_GUI_HOST");
  const int port = config.getInt("ENV_GUI_PORT");

  EnvMonitorMaster* master = new EnvMonitorMaster(db, comm);
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
          std::string nsmdata_name   = config.get("ENV_NSMDATA_NAME");
          std::string nsmdata_format = config.get("ENV_NSMDATA_FORMAT");
          const int nsmdata_revision = config.getInt("ENV_NSMDATA_REVSION");
          LogFile::debug("Env config (%d): %s = (%s:%s:%d)", count++, filename.c_str(),
                         nsmdata_name.c_str(), nsmdata_format.c_str(), nsmdata_revision);
          master->add(new NSMData(nsmdata_name, nsmdata_format, nsmdata_revision));
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
