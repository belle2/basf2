#include "daq/slc/runcontrol/RCCallback.h"

#include <daq/slc/database/ConfigObjectTable.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/nsm/NSMCommunicator.h>

using namespace Belle2;

RCCallback::RCCallback(const NSMNode& node) throw()
  : NSMCallback(node), m_state_demand(RCState::NOTREADY_S)
{
  add(RCCommand::LOAD);
  add(RCCommand::START);
  add(RCCommand::STOP);
  add(RCCommand::RECOVER);
  add(RCCommand::RESUME);
  add(RCCommand::PAUSE);
  add(RCCommand::ABORT);
  add(RCCommand::STATECHECK);
  add(RCCommand::TRIGFT);
  getNode().setState(RCState::NOTREADY_S);
}

bool RCCallback::perform(const NSMMessage& msg) throw()
{
  if (NSMCallback::perform(msg)) return true;
  const RCCommand cmd = msg.getRequestName();
  if (cmd.isAvailable(getNode().getState()) == NSMCommand::DISABLED) {
    return false;
  }
  NSMCommunicator* com = getCommunicator();
  RCState state = cmd.nextTState();
  bool result = true;
  if (state != Enum::UNKNOWN) {
    getNode().setState(state);
    LogFile::debug("%s >> %s (%s)", msg.getNodeName(),
                   cmd.getLabel(), state.getLabel());
    //com->replyOK(getNode());
    setReply("");
    if (cmd == RCCommand::LOAD) {
      result = preload(msg) && load();
    } else if (cmd == RCCommand::START) {
      result = start();
    } else if (cmd == RCCommand::STOP) {
      result = stop();
    } else if (cmd == RCCommand::RECOVER) {
      result = recover();
    } else if (cmd == RCCommand::RESUME) {
      result = resume();
    } else if (cmd == RCCommand::PAUSE) {
      result = pause();
    } else if (cmd == RCCommand::ABORT) {
      result = abort();
    } else if (cmd == RCCommand::TRIGFT) {
      result = trigft();
    }
  }
  if (result) {
    state = cmd.nextState();
    if (state != Enum::UNKNOWN)
      getNode().setState(state);
    com->replyOK(getNode());
  } else {
    com->replyError(getReply());
  }
  return true;
}

bool RCCallback::preload(const NSMMessage& msg) throw()
{
  if (msg.getNParams() < 1) {
    LogFile::debug("Loading method is not defined");
  } else if (msg.getParam(0) == NSMCommand::DBGET.getId()) {
    if (m_db == NULL) {
      setReply("Not ready for DB access");
      LogFile::error("Not ready for DB access");
      return false;
    } else {
      std::string runtype;
      if (msg.getLength() > 0) {
        runtype = msg.getData();
      } else {
        setReply("No runtype were given");
        LogFile::error("No runtype were given");
        return false;
      }
      try {
        m_db->connect();
        std::string nodename = getNode().getName();
        ConfigObjectTable table(m_db);
        m_config.setObject(table.get(runtype, nodename));
        m_db->close();
      } catch (const DBHandlerException& e) {
        setReply("DB access error");
        LogFile::error("DB access error");
        return false;
      }
    }
  } else if (msg.getParam(0) == NSMCommand::DBSET.getId()) {
    if (msg.getLength() > 0) {
      msg.getData(m_config.getObject());
    } else {
      setReply("No DB objects were given");
      LogFile::error("No DB objects were given");
      return false;
    }
  } else {
    LogFile::warning("No DB objects was loaded");
  }
  return true;
}
