#include "TemplateCallback.h"

#include <nsm/NSMNodeDaemon.h>

#include <base/Debugger.h>

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
  NSMNodeDaemon* daemon = new NSMNodeDaemon(node, callback);
  daemon->run();

  return 0;
}
