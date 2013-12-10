#include "daq/slc/apps/templated/TemplateCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/base/Debugger.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage : ./recvd <name>");
    return 1;
  }

  const char* name = argv[1];
  NSMNode* node = new NSMNode(name);
  TemplateCallback* callback = new TemplateCallback(node);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
  Belle2::debug("%s:%d", __FILE__, __LINE__);
  daemon->run();
  Belle2::debug("%s:%d", __FILE__, __LINE__);

  return 0;
}
