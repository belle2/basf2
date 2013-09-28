#include "ROCallback.hh"

#include <nsm/ROData.hh>
#include <nsm/NSMNodeDaemon.hh>

#include <node/RONode.hh>

#include <util/Debugger.hh>
#include <util/StringUtil.hh>

using namespace B2DAQ;

int main(int argc, char** argv)
{
  if (argc < 2) {
    B2DAQ::debug("Usage : ./recvd <name>");
    return 1;
  }
  const char* name = argv[1];
  //const char* path = argv[2];

  RONode* node = new RONode(name);
  ROData* data = new ROData(B2DAQ::form("%s_DATA", name));
  ROCallback* callback = new ROCallback(node, data);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(node, callback, data);
  node->setState(State::INITIAL_S);
  daemon->run();

  return 0;
}
