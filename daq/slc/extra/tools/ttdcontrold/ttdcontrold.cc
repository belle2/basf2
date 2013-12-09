#include "TTDCallback.hh"

#include <daq/slc/nsm/NSMNodeDaemon.hh>

#include <daq/slc/base/TTDNode.hh>

#include <daq/slc/base/Debugger.hh>
#include <daq/slc/base/StringUtil.hh>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage : ./ttdd <name>");
    return 1;
  }
  const char* name = argv[1];
  //const char* path = argv[2];

  TTDNode* node = new TTDNode(name);
  TTDCallback* callback = new TTDCallback(node);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(node, callback);
  daemon->run();

  return 0;
}
