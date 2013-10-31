#include "COPPERCallback.h"

#include <nsm/NSMData.h>
#include <nsm/NSMNodeDaemon.h>

#include <base/COPPERNode.h>
#include <base/Debugger.h>

#include <dlfcn.h>

using namespace Belle2;

typedef void* func_t(void*, const char*);

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage : ./cprcontrold <name>");
    return 1;
  }
  const char* name = argv[1];

  COPPERNode* node = new COPPERNode(name);
  COPPERCallback* callback = new COPPERCallback(node);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(node, callback);
  daemon->run();

  return 0;
}
