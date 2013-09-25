#include "RCCallback.hh"
#include "RCState.hh"

#include <nsm/NSMCommunicator.hh>

#include <util/Debugger.hh>

#include <belle2nsm.h>

using namespace B2DAQ;

bool RCCallback::perform(NSMMessage& msg) throw(NSMHandlerException)
{
  RCCommand cmd(msg.getRequestName());
  if (cmd == Command::OK) {
    return ok();
  } else if (cmd == Command::ERROR) {
    return error();
  }
  B2DAQ::debug("Node = %s", msg.getNodeName());
  B2DAQ::debug("Request = %s", msg.getRequestName());
  B2DAQ::debug("Command = %s", cmd.getLabel());
  B2DAQ::debug("State = %s", _node->getState().getLabel());
  if (cmd.isAvailable(_node->getState()) == 0) {
    return false;
  }
  setReply("");
  State state_org = _node->getState();
  bool result = false;
  if (cmd == RCCommand::BOOT) {
    result = boot();
  } else if (cmd == RCCommand::REBOOT) {
    result = reboot();
  } else if (cmd == RCCommand::LOAD) {
    result = load();
  } else if (cmd == RCCommand::RELOAD) {
    result = reload();
  } else if (cmd == RCCommand::START) {
    result = start();
  } else if (cmd == RCCommand::STOP) {
    result = stop();
  } else if (cmd == RCCommand::RESUME) {
    result = resume();
  } else if (cmd == RCCommand::PAUSE) {
    result = pause();
  } else if (cmd == RCCommand::STATECHECK) {
    result = stateCheck();
  } else if (cmd == RCCommand::ABORT) {
    result = abort();
  }
  NSMCommunicator* com = getCommunicator();
  if (result) {
    if (cmd != RCCommand::STATECHECK &&
        _node->getState() == state_org) {
      _node->setState(cmd.nextState());
    }
    com->replyOK(_node, _reply);
    return true;
  } else {
    _node->setState(RCState::ERROR_ES);
    com->replyError(_reply);
  }
  return false;
}

RCCallback::RCCallback(NSMNode* node) throw()
{
  _node = node;
  if (node != NULL) node->setState(RCState::INITIAL_S);
  add(RCCommand::BOOT);
  add(RCCommand::REBOOT);
  add(RCCommand::LOAD);
  add(RCCommand::RELOAD);
  add(RCCommand::START);
  add(RCCommand::STOP);
  add(RCCommand::RESUME);
  add(RCCommand::PAUSE);
  add(RCCommand::STATECHECK);
  add(RCCommand::RECOVER);
  add(RCCommand::ABORT);
}
