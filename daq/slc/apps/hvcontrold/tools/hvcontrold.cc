#include "daq/slc/apps/hvcontrold/HVMasterCallback.h"

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    HVMasterCallback* callback = new HVMasterCallback();
    ConfigFile config("slowcontrol", StringUtil::form("hvcontrol/%s", argv[1]));
    PostgreSQLInterface db(config.get("database.host"),
                           config.get("database.dbname"),
                           config.get("database.user"),
                           config.get("database.password"),
                           config.getInt("database.port"));
    const std::string nodename = config.get("nsm.nodename");
    LogFile::open("hvcontrold/" + nodename, LogFile::DEBUG);
    callback->getNode().setName(nodename);
    callback->setDB(config.get("dbtable"), &db);
    callback->setTimeout(config.getInt("hv.interval"));
    callback->setConfig(config.get("config"));
    NSMNodeDaemon(callback, config.get("nsm.host"),
                  config.getInt("nsm.port")).run();
  }
  return 0;
}

