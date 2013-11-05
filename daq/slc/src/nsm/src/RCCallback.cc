#include "nsm/RCCallback.h"
#include "nsm/NSMCommunicator.h"

#include "base/Debugger.h"
#include "base/ConfigFile.h"

using namespace Belle2;

bool RCCallback::perform(NSMMessage& msg) throw(NSMHandlerException)
{
  Command cmd(msg.getRequestName());
  if (cmd == Command::OK) {
    return ok();
  } else if (cmd == Command::ERROR) {
    return error();
  }
  if (cmd.isAvailable(_node->getState()) == 0) {
    return false;
  }
  setReply("");
  State state_org = _node->getState();
  bool result = false;
  NSMCommunicator* com = getCommunicator();
  if (cmd == Command::BOOT) {
    result = boot();
  } else if (cmd == Command::LOAD) {
    result = load();
  } else if (cmd == Command::START) {
    result = start();
  } else if (cmd == Command::STOP) {
    result = stop();
  } else if (cmd == Command::RECOVER) {
    result = recover();
  } else if (cmd == Command::RESUME) {
    result = resume();
  } else if (cmd == Command::PAUSE) {
    result = pause();
  } else if (cmd == Command::ABORT) {
    result = abort();
  } else if (cmd == Command::TRIGFT) {
    result = trigft();
  } else if (cmd == Command::SETPARAMS) {
    result = setparams();
  } else if (cmd == Command::STATECHECK) {
    com->replyOK(_node, "");
    return true;
  }
  if (result) {
    if (cmd != Command::TRIGFT &&
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
  add(Command::RECOVER);
  //add(Command::RESUME);
  //add(Command::PAUSE);
  //add(Command::ABORT);
  add(Command::STATECHECK);
  add(Command::SETPARAMS);
}

bool RCCallback::setparams() throw()
{
  ConfigFile config(_node->getName());
  _node->getData()->readValueString(config);
  return true;
}

