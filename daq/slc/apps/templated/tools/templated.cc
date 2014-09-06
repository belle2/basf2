#include "daq/slc/apps/templated/TemplateCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <cstdlib>
#include <unistd.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 5) {
    LogFile::debug("Usage : %s <name> <format> <revision> <nsmhost> <nsmport>", argv[0]);
    return 1;
  }
  //daemon(0, 0);
  LogFile::open("rocontrold");
  const char* name = argv[1];
  const char* format = argv[2];
  const int revision = atoi(argv[3]);
  const char* hostname = argv[4];
  const int port = atoi(argv[5]);
  NSMNode node(name);
  TemplateCallback* callback = new TemplateCallback(node, format, revision);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback, hostname, port);
  daemon->run();

  return 0;
}
