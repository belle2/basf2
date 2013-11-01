#include "ROCallback.h"

#include <nsm/NSMNodeDaemon.h>

#include <base/RONode.h>

#include <base/Debugger.h>
#include <base/StringUtil.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage : ./rocontrold <name>");
    return 1;
  }
  const char* name = argv[1];

  RONode* node = new RONode(name);
  ROCallback* callback = new ROCallback(node);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(node, callback);
  daemon->run();

  return 0;
}
