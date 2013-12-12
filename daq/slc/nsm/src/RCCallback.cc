#include "daq/slc/nsm/RCCallback.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/system/TCPSocketWriter.h"
#include "daq/slc/system/TCPSocketReader.h"

#include "daq/slc/base/ConfigFile.h"
#include "daq/slc/base/Debugger.h"

#include <iostream>

using namespace Belle2;

RCCallback::RCCallback(NSMNode* node) throw()
  : NSMCallback(node)
{
  add(Command::BOOT);
  add(Command::LOAD);
  add(Command::START);
  add(Command::STOP);
  add(Command::RECOVER);
  //add(Command::RESUME);
  //add(Command::PAUSE);
  add(Command::ABORT);
  add(Command::STATE);
  add(Command::STATECHECK);
  add(Command::TRIGFT);
  ConfigFile config("slowcontrol");
}

void RCCallback::download()
{
  if (_node->getData() != NULL) {
    TCPSocket socket;
    try {
      ConfigFile config;
      int port = config.getInt("RC_LOCAL_PORT");
      if (port <= 0) return;
      socket.connect(config.get("RC_LOCAL_HOST"),
                     port);
      TCPSocketWriter writer(socket);
      TCPSocketReader reader(socket);
      writer.writeString(_node->getName());
      _node->getData()->readObject(reader);
    } catch (const IOException& e) {
      Belle2::debug("Socket Erorr: %s", e.what());
    }
    socket.close();
  }
}

bool RCCallback::perform(const Command& cmd, NSMMessage&)
throw(NSMHandlerException)
{
  setReply("");
  if (cmd == Command::OK) {
    return ok();
  } else if (cmd == Command::ERROR) {
    return error();
  }
  if (cmd.isAvailable(_node->getState()) == 0) {
    return false;
  }
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
  } else if (cmd == Command::STATECHECK) {
    if (com != NULL) com->replyOK(_node, "");
    return true;
  } else if (cmd == Command::STATE) {
    return true;
  }
  if (result) {
    if (cmd != Command::TRIGFT && _node->getState() == state_org) {
      _node->setState(cmd.nextState());
    }
    if (com != NULL) com->replyOK(_node, _reply);
    return true;
  } else {
    _node->setState(State::ERROR_ES);
    if (com != NULL) com->replyError(_reply);
  }
  return false;
}

