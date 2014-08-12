#include "daq/slc/apps/storagerd/StoragerCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <cstdlib>
#include <unistd.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("Usage : %s <name>", argv[0]);
    return 1;
  }
  //daemon(0, 0);
  LogFile::open("storage");
  const char* name = argv[1];
  NSMNode node(name);
  StoragerCallback* callback = new StoragerCallback(node);
  callback->setFile("storage");
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
  daemon->run();

  return 0;
}
