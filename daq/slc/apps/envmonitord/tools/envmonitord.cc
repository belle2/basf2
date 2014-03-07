#include "daq/slc/apps/envmonitord/EnvMonitorMaster.h"
#include "daq/slc/apps/envmonitord/DataSender.h"

#include <daq/slc/apps/SocketAcceptor.h>

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/system/PThread.h>
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
  DBInterface* db = new PostgreSQLInterface(config.get("database.host"),
                                            config.get("database.dbname"),
                                            config.get("database.user"),
                                            config.get("database.password"),
                                            config.getInt("database.port"));
  const std::string config_path = config.get("env.config.dir");
  const std::string hostname = config.get("env.server.host");
  const int port = config.getInt("env.server.port");

  EnvMonitorMaster* master = new EnvMonitorMaster(db, comm);
  std::vector<std::string> file_v = Belle2::split(config.get("env.config.files"), ',');
  int count = 0;
  for (size_t i = 0; i < file_v.size(); i++) {
    std::string filename = file_v[i] + ".conf";
    config.clear();
    config.read(config_path + "/" + filename);
    std::string nsmdata_name   = config.get("nsm.data.name");
    std::string nsmdata_format = config.get("nsm.data.format");
    const int nsmdata_revision = config.getInt("nsm.data.revision");
    if (nsmdata_name.size() > 0 && nsmdata_format.size() > 0 && nsmdata_revision > 0) {
      LogFile::debug("Env config (%d): %s = (%s:%s:%d)", count++, filename.c_str(),
                     nsmdata_name.c_str(), nsmdata_format.c_str(), nsmdata_revision);
      master->add(new NSMData(nsmdata_name, nsmdata_format, nsmdata_revision));
    }
  }
  PThread(new EnvUIAcceptor(hostname, port, master));
  LogFile::debug("Start socket acception from GUIs");
  master->run();
  return 0;
}
