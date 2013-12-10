#include "COPPERCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/xml/ObjectLoader.h>

#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

typedef void* func_t(void*, const char*);

int main(int argc, char** argv)
{
  if (argc < 3) {
    Belle2::debug("Usage : ./cprcontrold <name> <config>");
    return 1;
  }
  const char* name = argv[1];
  const char* config = argv[2];
  NSMNode* node = new NSMNode(name);
  COPPERCallback* callback = new COPPERCallback(node, config);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
  daemon->run();

  return 0;
}
