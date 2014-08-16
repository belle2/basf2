#include "daq/slc/apps/cprcontrold/COPPERCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("Usage : ./cprcontrold <name>");
    return 1;
  }
  //daemon(0, 0);
  const std::string name = argv[1];
  LogFile::open("cprcontrold_" + name);
  NSMNode node(name);
  COPPERCallback* callback = new COPPERCallback(node, NULL);
  callback->setFilePath("database/copper");
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
  daemon->run();

  return 0;
}
