#include "DataReceiverCallback.hh"

#include <nsm/DataReceiverData.hh>
#include <nsm/NSMNodeDaemon.hh>

#include <runcontrol/RCState.hh>

#include <node/DataReceiverNode.hh>

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

  DataReceiverNode* node = new DataReceiverNode(name);
  DataReceiverData* data = new DataReceiverData(B2DAQ::form("%s_DATA", name));
  DataReceiverCallback* callback = new DataReceiverCallback(node, data);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(node, callback, data);
  node->setState(RCState::INITIAL_S);
  daemon->run();

  return 0;
}
