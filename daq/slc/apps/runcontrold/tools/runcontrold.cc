#include "daq/slc/apps/runcontrold/RunControlMasterCallback.h"

#include <daq/slc/runcontrol/RCNodeDaemon.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    int port = config.getInt("provider.port");
    RunControlCallback* callback = new RunControlCallback(port);
    callback->setPriorityToDB(LogFile::getPriority(config.get("log.priority.db")));
    callback->setPriorityToGlobal(LogFile::getPriority(config.get("log.priority.global")));
    callback->setExcludedNodes(StringUtil::split(config.get("node.excluded"), ','));
    RCCallback* callback2 = new RunControlMasterCallback(callback);
    RCNodeDaemon(config, callback, callback2).run();
  }
  return 0;
}

