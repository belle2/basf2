#include "daq/slc/apps/logger/LoggerCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage : ./loggerd <config>");
    return 1;
  }
  ConfigFile config("slowcontrol", argv[1]);
  NSMCommunicator* comm = new NSMCommunicator(new NSMNode(Belle2::toupper(argv[1]) + "_LOGGER"));
  comm->init(config.get("NSM_GLOBAL_HOST"),
             config.getInt("NSM_GLOBAL_PORT"));
  NSMNode* node = new NSMNode(config.get("LOG_NSM_NAME"));
  LoggerCallback* callback = new LoggerCallback(node, comm);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback,
                                            config.get("NSM_LOCAL_HOST"),
                                            config.getInt("NSM_LOCAL_PORT"));
  daemon->run();

  return 0;
}
