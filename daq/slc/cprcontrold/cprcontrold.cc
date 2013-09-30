#include "COPPERCallback.hh"

#include <nsm/NSMData.hh>
#include <nsm/NSMNodeDaemon.hh>

#include <node/COPPERNode.hh>

#include <util/Debugger.hh>
#include <util/StringUtil.hh>

#include <dlfcn.h>

using namespace B2DAQ;

typedef void* func_t(void*, const char*);

int main(int argc, char** argv)
{
  if (argc < 2) {
    B2DAQ::debug("Usage : ./cprcontrold <name>");
    return 1;
  }
  const char* name = argv[1];

  COPPERNode* node = new COPPERNode(name);
  COPPERCallback* callback = new COPPERCallback(node);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(node, callback);
  daemon->run();

  return 0;
}
