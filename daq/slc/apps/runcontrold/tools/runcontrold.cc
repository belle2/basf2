#include "daq/slc/apps/runcontrold/RunControlMasterCallback.h"
#include "daq/slc/apps/runcontrold/RunControlCallback.h"

#include <daq/slc/psql/PostgreSQLInterface.h>

#include <daq/slc/runcontrol/RCNodeDaemon.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    RunControlCallback* callback = new RunControlCallback();
    callback->setPriorityToDB(LogFile::getPriority(config.get("log.priority.db")));
    callback->setPriorityToGlobal(LogFile::getPriority(config.get("log.priority.global")));
    //callback->setExcludedNodes(StringUtil::split(config.get("node.excluded"), ','));
    callback->setLocalRunControls(StringUtil::split(config.get("localruncontrols"), ','));
    //callback->setLogTable(config.get("log.dbtable"));
    callback->setRestartTime(config.getInt("restarttime"));
    RCCallback* callback2 = new RunControlMasterCallback(callback);
    RCNodeDaemon(config, callback, callback2,
                 new PostgreSQLInterface(config.get("database.host"),
                                         config.get("database.dbname"),
                                         config.get("database.user"),
                                         config.get("database.password"),
                                         config.getInt("database.port"))).run();
  }
  return 0;
}

