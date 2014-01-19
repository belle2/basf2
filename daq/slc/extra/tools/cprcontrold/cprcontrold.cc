#include "COPPERCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/xml/ObjectLoader.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <unistd.h>

using namespace Belle2;

typedef void* func_t(void*, const char*);

int main(int argc, char** argv)
{
  if (argc < 3) {
    Belle2::debug("Usage : ./cprcontrold <name> <config>");
    return 1;
  }
  daemon(0, 0);
  const char* name = argv[1];
  const char* config = argv[2];
  LogFile::open(Belle2::form("cprcontrol_%s", name));
  NSMNode* node = new NSMNode(name);
  COPPERCallback* callback = new COPPERCallback(node, config);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
  daemon->run();

  return 0;
}
