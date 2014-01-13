#include "daq/slc/apps/rocontrold/ROCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>

#include <cstdlib>
#include <unistd.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    Belle2::debug("Usage : ./rocontrold <name> <config>");
    return 1;
  }
  daemon(0, 0);
  LogFile::open("rocontrold");
  const char* name = argv[1];
  const char* config = argv[2];
  NSMNode* node = new NSMNode(name);
  ROCallback* callback = new ROCallback(node, config);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
  daemon->run();

  return 0;
}
