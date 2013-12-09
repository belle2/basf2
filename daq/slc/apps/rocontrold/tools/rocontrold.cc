#include "daq/slc/apps/rocontrold/ROCallback.h"

#include <daq/slc/base/Debugger.h>
#include <daq/slc/nsm/NSMNodeDaemon.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage : ./rocontrold <name>");
    return 1;
  }
  const char* name = argv[1];
  NSMNode* node = new NSMNode(name);
  ROCallback* callback = new ROCallback(node);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
  daemon->run();

  return 0;
}
