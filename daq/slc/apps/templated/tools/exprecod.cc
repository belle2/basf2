#include "daq/slc/apps/exprecod/ExpRecoCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/base/Debugger.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage : ./exprecod <name>");
    return 1;
  }

  const char* name = argv[1];
  NSMNode* node = new NSMNode(name);
  ExpRecoCallback* callback = new ExpRecoCallback(node);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
  daemon->run();

  return 0;
}
