#include "LoggerCallback.h"

#include <nsm/NSMNodeDaemon.h>
#include <nsm/NSMCommunicator.h>

#include <base/Debugger.h>
#include <base/ConfigFile.h>

#include <iostream>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage : ./loggerd <name> <ip> <port>");
    return 1;
  }
  NSMCommunicator* comm = new NSMCommunicator(new NSMNode(argv[1]));
  comm->init(argv[2], atoi(argv[3]));

  ConfigFile config("slc_config");
  NSMNode* node = new NSMNode(config.get("LOG_NSM_NAME"));
  LoggerCallback* callback = new LoggerCallback(node, comm);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(node, callback, argv[2], 8322);
  daemon->run();

  return 0;
}
