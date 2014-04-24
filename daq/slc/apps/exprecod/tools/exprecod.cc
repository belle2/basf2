#include "daq/slc/apps/exprecod/ExpRecoCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("Usage : %s <name>", argv[0]);
    return 1;
  }

  const char* name = argv[1];
  NSMNode node(name);
  ExpRecoCallback* callback = new ExpRecoCallback(node);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
  daemon->run();

  return 0;
}
