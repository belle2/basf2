#include "TTDCallback.hh"

#include <nsm/TTDData.hh>
#include <nsm/NSMNodeDaemon.hh>

#include <node/TTDNode.hh>

#include <util/Debugger.hh>
#include <util/StringUtil.hh>

using namespace B2DAQ;

int main(int argc, char** argv)
{
  if (argc < 2) {
    B2DAQ::debug("Usage : ./ttdd <name>");
    return 1;
  }
  const char* name = argv[1];
  //const char* path = argv[2];

  TTDNode* node = new TTDNode(name);
  TTDData* data = new TTDData(B2DAQ::form("%s_DATA", name));
  TTDCallback* callback = new TTDCallback(node, data);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(node, callback, data);
  daemon->run();

  return 0;
}
