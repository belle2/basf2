#include "daq/slc/runcontrol/RCCallback.h"

#include <daq/slc/database/ConfigObjectTable.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/TCPSocketWriter.h>

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

void RCCallback::sendPause() throw()
{
  NSMCommunicator* com = getCommunicator();
  if (com != NULL) {
    sendPause(com->getMaster());
  }
}

void RCCallback::sendPause(const NSMNode& node) throw()
{
  try {
    NSMCommunicator* com = getCommunicator();
    if (com != NULL && node.getName().size() > 0) {
      com->sendRequest(NSMMessage(node, RCCommand::PAUSE,
                                  getNode().getState().getLabel()));
    }
  } catch (const NSMHandlerException& e) {
  }
}

bool RCCallback::perform(const NSMMessage& msg) throw()
{
  //LogFile::debug("%s >> %s", msg.getNodeName(), msg.getData());
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
    getNode().setError(0);
    state = cmd.nextState();
    if (state != Enum::UNKNOWN)
      getNode().setState(state);
    com->replyOK(getNode());
  } else {
    if (getNode().getError() == 0) {
      getNode().setError(255);
    }
    com->replyError(getNode().getError(), getReply());
  }
  return true;
}

bool RCCallback::preload(const NSMMessage& msg) throw()
{
  if (msg.getNParams() < 1) {
    if (m_tablename.size() > 0) {
      LogFile::warning("Loading object from file : %s", m_tablename.c_str());
      ConfigObject obj = DBObjectLoader::load(m_path, m_tablename, true);
      m_config.setObject(obj);
    } else {
      LogFile::debug("Loading method is not defined");
    }
  } else if (msg.getParam(0) == NSMCommand::DBGET.getId()) {
    if (msg.getNParams() > 3 && msg.getParam(2) > 0) {
      TCPSocket socket(getCommunicator()->getNodeHost(), msg.getParam(2));
      try {
        socket.connect();
        TCPSocketWriter writer(socket);
        writer.writeInt(msg.getParam(3));
        TCPSocketReader reader(socket);
        m_config.getObject().readObject(reader);
      } catch (const IOException& e) {
        LogFile::error(e.what());
        socket.close();
        return false;
      }
      socket.close();
    } else {
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
