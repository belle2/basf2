#include "daq/slc/nsm/RCCallback.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/system/TCPSocketWriter.h"
#include "daq/slc/system/TCPSocketReader.h"
#include "daq/slc/system/LogFile.h"

#include "daq/slc/base/State.h"
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
  add(Command::RESUME);
  add(Command::PAUSE);
  add(Command::ABORT);
  add(Command::STATE);
  add(Command::STATECHECK);
  add(Command::TRIGFT);
  if (node != NULL)
    node->setState(State::INITIAL_S);
}

void RCCallback::download()
{
  if (_node != NULL && _node->getData() != NULL) {
    TCPSocket socket;
    try {
      ConfigFile config;
      int port = config.getInt("RC_LOCAL_PORT");
      std::string host = config.get("RC_LOCAL_HOST");
      if (port <= 0 || host.size() == 0) return;
      socket.connect(host, port);
      TCPSocketWriter writer(socket);
      TCPSocketReader reader(socket);
      writer.writeString(_node->getName());
      _node->getData()->readObject(reader);
    } catch (const IOException& e) {
      LogFile::debug("Socket Erorr: %s", e.what());
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
    LogFile::debug("ERROR");
    return error();
  } else if (cmd == Command::FATAL) {
    LogFile::debug("FATAL");
    return fatal();
  }
  if (cmd.isAvailable(_node->getState()) == 0) {
    return false;
  }
  State state_org = _node->getState();
  bool result = false;
  NSMCommunicator* com = getCommunicator();
  if (cmd == Command::BOOT) {
    LogFile::debug("BOOT");
    result = boot();
  } else if (cmd == Command::LOAD) {
    LogFile::debug("LOAD");
    result = load();
  } else if (cmd == Command::START) {
    LogFile::debug("START");
    result = start();
  } else if (cmd == Command::STOP) {
    LogFile::debug("STOP");
    result = stop();
  } else if (cmd == Command::RECOVER) {
    LogFile::debug("RECOVER");
    result = recover();
  } else if (cmd == Command::RESUME) {
    LogFile::debug("RESUME");
    result = resume();
  } else if (cmd == Command::PAUSE) {
    LogFile::debug("PAUSE");
    result = pause();
  } else if (cmd == Command::ABORT) {
    LogFile::debug("ABORT");
    result = abort();
  } else if (cmd == Command::TRIGFT) {
    LogFile::debug("TRGFT");
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

