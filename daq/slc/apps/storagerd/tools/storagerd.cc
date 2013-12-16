#include "daq/slc/apps/storagerd/StoragerCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage : ./storagerd <name>");
    return 1;
  }
  const char* name = argv[1];
  NSMNode* node = new NSMNode(name);
  ConfigFile config("slowcontrol", "storage");
  const std::string dir = Belle2::form("%s/daq/storage/examples/",
                                       getenv("BELLE2_LOCAL_DIR"));
  StoragerCallback* callback = new StoragerCallback(node, dir);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
  daemon->run();

  return 0;
}
