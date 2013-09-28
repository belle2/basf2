#include "RCCallback.hh"

#include <nsm/NSMCommunicator.hh>

#include <util/Debugger.hh>

#include <belle2nsm.h>

using namespace B2DAQ;

bool RCCallback::perform(NSMMessage& msg) throw(NSMHandlerException)
{
  Command cmd(msg.getRequestName());
  if (cmd == Command::OK) {
    return ok();
  } else if (cmd == Command::ERROR) {
    return error();
  }
  /*
  B2DAQ::debug("Node = %s", msg.getNodeName());
  B2DAQ::debug("Request = %s", msg.getRequestName());
  B2DAQ::debug("Command = %s", cmd.getLabel());
  B2DAQ::debug("State = %s", _node->getState().getLabel());
  */
  if (cmd.isAvailable(_node->getState()) == 0) {
    return false;
  }
  setReply("");
  State state_org = _node->getState();
  bool result = false;
  if (cmd == Command::BOOT) {
    result = boot();
  } else if (cmd == Command::LOAD) {
    result = load();
  } else if (cmd == Command::START) {
    result = start();
  } else if (cmd == Command::STOP) {
    result = stop();
  } else if (cmd == Command::RESUME) {
    result = resume();
  } else if (cmd == Command::PAUSE) {
    result = pause();
  } else if (cmd == Command::ABORT) {
    result = abort();
  }
  NSMCommunicator* com = getCommunicator();
  if (result) {
    if (cmd != Command::STATECHECK &&
        _node->getState() == state_org) {
      _node->setState(cmd.nextState());
    }
    com->replyOK(_node, _reply);
    return true;
  } else {
    _node->setState(State::ERROR_ES);
    com->replyError(_reply);
  }
  return false;
}

RCCallback::RCCallback(NSMNode* node) throw()
{
  _node = node;
  if (node != NULL) node->setState(State::INITIAL_S);
  add(Command::BOOT);
  add(Command::LOAD);
  add(Command::START);
  add(Command::STOP);
  add(Command::RESUME);
  add(Command::PAUSE);
  add(Command::ABORT);
}
