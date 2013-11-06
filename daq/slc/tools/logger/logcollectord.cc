#include "LogCollectorCallback.h"

#include "SocketAcceptor.h"

#include <nsm/NSMNodeDaemon.h>
#include <nsm/NSMCommunicator.h>

#include <base/Debugger.h>
#include <base/ConfigFile.h>

#include <system/PThread.h>

#include <iostream>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 1) {
    Belle2::debug("Usage : ./loggerd");
    return 1;
  }
  ConfigFile config("slc_config");
  PThread(new SocketAcceptor(config.get("LOG_GUI_HOST"),
                             config.getInt("LOG_GUI_PORT")));
  NSMNode* node = new NSMNode(config.get("LOG_NSM_NAME"));
  LogCollectorCallback* callback =
    new LogCollectorCallback(node);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(node, callback);
  daemon->run();

  return 0;
}
