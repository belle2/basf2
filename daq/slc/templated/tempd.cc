#include "TemplateCallback.hh"

#include <nsm/NSMNodeDaemon.hh>

#include <runcontrol/RCState.hh>

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

  NSMNode* node = new NSMNode(name);
  TemplateCallback* callback = new TemplateCallback(node);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(node, callback);
  node->setState(RCState::INITIAL_S);
  daemon->run();

  return 0;
}
