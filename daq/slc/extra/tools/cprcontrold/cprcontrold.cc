#include "COPPERCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <unistd.h>

using namespace Belle2;

typedef void* func_t(void*, const char*);

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("Usage : ./cprcontrold <name> <config>");
    return 1;
  }
  daemon(0, 0);
  const std::string name = argv[1];
  const std::string config = argv[2];
  LogFile::open("cprcontrold_" + name);
  NSMNode node(name);
  COPPERCallback* callback = new COPPERCallback(node, config);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
  daemon->run();

  return 0;
}
