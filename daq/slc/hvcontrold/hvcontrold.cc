#include "ArichHVCallback.h"
#include "ArichHVCommunicator.h"

#include <nsm/NSMNodeDaemon.h>
#include <nsm/NSMData.h>

#include <system/PThread.h>

#include <base/Debugger.h>

#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 4) {
    Belle2::debug("Usage : ./recvd <name> <host> <port>");
    return 1;
  }

  const std::string name = argv[1];
  const std::string host = argv[2];
  const int port = atoi(argv[3]);
  NSMNode* node = new NSMNode(name);
  NSMData* data = new NSMData(name + "_STATUS", "ArichHVStatus", 1);
  ArichHVCallback* callback = new ArichHVCallback(node);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
  PThread(new ArichHVCommunicator(callback, host, port, data));
  daemon->run();

  return 0;
}
