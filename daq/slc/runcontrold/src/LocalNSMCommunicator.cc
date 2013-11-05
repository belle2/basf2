#include "LocalNSMCommunicator.h"
#include "MessageBox.h"

#include "nsm/NSMCommunicator.h"

#include "base/Debugger.h"

#include <cstdlib>

using namespace Belle2;

LocalNSMCommunicator::LocalNSMCommunicator(NSMCommunicator* gcomm,
                                           NSMCommunicator* lcomm)
throw (NSMHandlerException)
{
  _comm_v.push_back(gcomm);
  _comm_v.push_back(lcomm);
}

int LocalNSMCommunicator::sendMessage(NSMNode* node,
                                      RunControlMessage& message)
throw(IOException)
{
  int id;
  if (_comm_m.find(node) == _comm_m.end()) {
    for (size_t i = 0; i < _comm_v.size(); i++) {
      id = node->getNodeID();
      if (id < 0) id = _comm_v[i]->getNodeIdByName(node->getName());
      int pid = (id >= 0) ? _comm_v[i]->getNodePidByName(node->getName()) : -1;
      if (id >= 0 && pid > 0)
        _comm_m.insert(std::map<NSMNode*, NSMCommunicator*>::value_type(node, _comm_v[i]));
    }
  }
  if (_comm_m.find(node) != _comm_m.end()) {
    if (!(message.getCommand() == Command::STATECHECK &&
          node->getState() == State::RUNNING_S)) {
      _comm_m[node]->sendRequest(node, message.getCommand(), message.getMessage());
      node->setConnection(Connection::ONLINE);
    }
    return id;
  } else {
    node->setConnection(Connection::OFFLINE);
    node->setState(State::UNKNOWN);
    return -1;
  }
}

void LocalNSMCommunicator::run()
{
  try {
    for (size_t i = 0; i < _comm_v.size(); i++)
      if (_comm_v[i] != NULL) _comm_v[i]->init();
  } catch (const IOException& e) {
    Belle2::debug("[FATAL] %s:%d:Failed to NSM init %s", __FILE__, __LINE__, e.what());
    exit(1);
  }
  try {
    while (true) {
      RunControlMessage msg(RunControlMessage::LOCALNSM);
      NSMCommunicator* comm = NSMCommunicator::select(5000);
      if (comm != NULL) {
        msg.setMessage(comm->getMessage());
        msg.setCommand(msg.getMessage().getRequestName());
      } else {
        msg.setCommand(Command::STATECHECK);
      }
      MessageBox::get().push(msg);
    }
  } catch (const IOException& e) {
  }
}
