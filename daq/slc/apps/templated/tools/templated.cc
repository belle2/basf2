#include "daq/slc/apps/templated/TemplateCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <cstdlib>
#include <unistd.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 4) {
    LogFile::debug("Usage : %s <name> <host> <port>", argv[0]);
    return 1;
  }
  //daemon(0, 0);
  LogFile::open("rocontrold");
  const char* name = argv[1];
  const char* host = argv[2];
  const char* port = argv[3];
  NSMNode node(name);
  TemplateCallback* callback = new TemplateCallback(node, host, port);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
  daemon->run();

  return 0;
}
