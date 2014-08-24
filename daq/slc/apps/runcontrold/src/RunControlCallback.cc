#include "daq/slc/apps/runcontrold/RunControlCallback.h"

#include <daq/slc/apps/runcontrold/RunSetting.h>
#include <daq/slc/apps/runcontrold/RunSummary.h>
#include <daq/slc/apps/runcontrold/rc_status.h>

#include <daq/slc/readout/ronode_status.h>

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/LoggerObjectTable.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/TCPSocketWriter.h>

#include <daq/slc/base/ConfigFile.h>

#include <cstring>
#include <sstream>
#include <unistd.h>

using namespace Belle2;

RunControlCallback::RunControlCallback(const NSMNode& node,
                                       const std::string& runtype,
                                       int port)
  : RCCallback(node), m_setting(node)
{
  add(RCCommand::EXCLUDE);
  add(RCCommand::INCLUDE);
  m_data = NSMData(node.getName() + "_STATUS",
                   "rc_status", rc_status_revision);
  m_runtype_default = runtype;
  m_port = port;
  m_callback = NULL;
}

void RunControlCallback::init() throw()
{
  NSMMessage msg(getNode(), NSMCommand::DBGET, m_runtype_default);
  msg.setNParams(1);
  msg.setParam(0, NSMCommand::DBGET.getId());
  msg.setData(m_runtype_default);
  preload(msg);
  {
    ConfigObjectList& obj_v(getConfig().getObject().getObjects("nsmdata"));
    for (size_t i = 0; i < obj_v.size(); i++) {
      ConfigObject& obj(obj_v[i]);
      NSMData data(obj.getText("dataname"), obj.getText("format"),
                   obj.getInt("file_revision"));
      m_data_v.push_back(data);
    }
  }
  rc_status* status = (rc_status*)m_data.allocate(getCommunicator());
  status->state = getNode().getState().getId();
  status->configid = getConfig().getObject().getId();
  {
    ConfigObjectList& obj_v(getConfig().getObject().getObjects("node"));
    status->nnodes = obj_v.size();
    for (size_t i = 0; i < obj_v.size(); i++) {
      DBObject& obj(obj_v[i].getObject("runtype"));
      m_node_v.push_back(NSMNode(obj.getNode()));
    }
  }
  getNode().setState(Enum::UNKNOWN);
  timeout();
  if (m_port > 0) {
    PThread(new ConfigProvider(this, "0.0.0.0", m_port));
  }
}

void RunControlCallback::update() throw()
{
  rc_status* status = (rc_status*)m_data.get();
  ConfigObjectList& obj_v(getConfig().getObject().getObjects("node"));
  for (size_t i = 0; i < m_data_v.size(); i++) {
    NSMData& data(m_data_v[i]);
    if (!data.isAvailable()) {
      try {
        data.open(getCommunicator());
        LogFile::debug("Opened nsm data : %s ", data.getName().c_str());
      } catch (const NSMHandlerException& e) {
        LogFile::warning("Failed to open nsm data : %s", data.getName().c_str());
      }
    }
  }
  RCState state(m_node_v[0].getState());
  for (size_t i = 0; i < m_node_v.size(); i++) {
    status->node[i].error = m_node_v[i].getError();
    status->node[i].state = m_node_v[i].getState().getId();
    DBObject& obj(obj_v[i].getObject("runtype"));
    status->node[i].configid = obj.getId();
    if (state != m_node_v[i].getState()) {
      state = RCState::UNKNOWN;
    }
  }
  if (state != RCState::UNKNOWN) {
    getNode().setState(state);
  }
  status->state = getNode().getState().getId();
  status->configid = getConfig().getObject().getId();
  status->nnodes = m_node_v.size();
}

void RunControlCallback::timeout() throw()
{
  NSMCommunicator& com(*getCommunicator());
  for (size_t i = 0; i < m_node_v.size(); i++) {
    NSMNode& node(m_node_v[i]);
    RCState state(node.getState());
    if (!com.isConnected(node)) {
      node.setState(Enum::UNKNOWN);
      continue;
    }
    if (!state.isStable()) {
      try {
        LogFile::debug("STATECHECK >> %s", node.getName().c_str());
        com.sendRequest(NSMMessage(node, RCCommand::STATECHECK));
      } catch (const NSMHandlerException& e) {
        LogFile::warning("Failed to send statecheck : %s",
                         node.getName().c_str());
      }
    }
  }
  update();
}

bool RunControlCallback::log() throw()
{
  NSMMessage& msg(getMessage());
  if (msg.getLength() > 0) {
    StringList slist = StringUtil::split(msg.getData(), '\n');
    std::string nodename = slist[0];
    std::stringstream ss;
    for (size_t i = 1; i < slist.size(); i++) {
      ss << slist[i];
      if (i < slist.size() - 1) ss << "\n";
    }
    DAQLogMessage log(nodename, (LogFile::Priority)msg.getParam(0),
                      ss.str(), Date(msg.getParam(1)));
    log.setNode(getNode().getName());
    try {
      getDB()->connect();
      LoggerObjectTable(getDB()).add(log, true);
      getDB()->close();
      if (log.getPriority() > LogFile::INFO) {
        NSMCommunicator* com = getCommunicator();
        com->sendLog(log);
        if (m_callback && m_callback->getCommunicator()) {
          NSMCommunicator* com_g = m_callback->getCommunicator();
          com_g->sendLog(log);
        }
      }
    } catch (const DBHandlerException& e) {
      LogFile::error("DB errir : %s", e.what());
    }
  }
  return true;
}

bool RunControlCallback::perform(const NSMMessage& msg) throw()
{
  if (NSMCallback::perform(msg)) return true;
  const RCCommand cmd = msg.getRequestName();
  if (cmd.isAvailable(getNode().getState()) == NSMCommand::DISABLED) {
    return false;
  }
  if (cmd == RCCommand::STATECHECK) {
    NSMCommunicator& com(*getCommunicator());
    com.replyOK(getNode());
    if (m_callback && m_callback->getCommunicator()) {
      NSMCommunicator& com_g(*m_callback->getCommunicator());
      com_g.replyOK(getNode());
    }
    return true;
  }
  RCState state = cmd.nextTState();
  bool result = true;
  if (state != Enum::UNKNOWN) {
    getNode().setState(state);
    LogFile::debug("%s >> %s (%s)", msg.getNodeName(),
                   cmd.getLabel(), state.getLabel());
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
  if (!result) {
    NSMCommunicator& com(*getCommunicator());
    com.replyError(getNode().getError(), getReply());
    if (m_callback && m_callback->getCommunicator()) {
      NSMCommunicator& com_g(*m_callback->getCommunicator());
      com_g.replyError(getNode().getError(), getReply());
    }
  }
  return true;
}


bool RunControlCallback::ok() throw()
{
  NSMMessage& msg(getMessage());
  if (msg.getLength() > 0) {
    RCState state(msg.getData());
    const std::string nodename = msg.getNodeName();
    NSMNodeIterator it = find(nodename);
    if (state == NSMState::UNKNOWN) {
      LogFile::error("got unknown state (%s) from %s",
                     msg.getData(), nodename.c_str());
    } else if (it != m_node_v.end()) {
      NSMCommunicator& com(*getCommunicator());
      NSMNode& node(*it);
      node.setState(state);
      com.sendState(node);
      if (m_callback && m_callback->getCommunicator()) {
        NSMCommunicator& com_g(*m_callback->getCommunicator());
        com_g.sendState(node);
      }
      if (state == RCState::PAUSED_S) {
        const size_t nobj = getConfig().getObject().getNObjects("node");
        for (size_t i = 0; i < nobj; i++) {
          if (m_node_v[i].getState() == RCState::RECOVERING_RS) {
            try {
              com.sendRequest(NSMMessage(m_node_v[i],
                                         RCCommand::RECOVER));
            } catch (const IOException& e) {
              LogFile::warning(e.what());
            }
          }
        }
      }
      if (state.isStable() && synchronize(node)) {
        RCState state_org(getNode().getState());
        if (state != state_org) {
          getNode().setState(state);
          LogFile::debug("%s >> OK (%s) (RC=%s) %s",
                         nodename.c_str(), msg.getData(),
                         getNode().getState().getLabel(), state.getLabel());
          com.replyOK(getNode());
          if (m_callback && m_callback->getCommunicator()) {
            NSMCommunicator& com_g(*m_callback->getCommunicator());
            com_g.replyOK(getNode());
          }
        }
      }
      update();
    }
  }
  return true;
}

bool RunControlCallback::error() throw()
{
  NSMMessage& msg(getMessage());
  if (msg.getLength() > 0) {
    std::string nodename = msg.getNodeName();
    LogFile::error("Error from '%s' (%s)",
                   nodename.c_str(), msg.getData());
    DAQLogMessage log(nodename, LogFile::ERROR,
                      msg.getData(), Date());
    log.setNode(getNode().getName());
    getDB()->connect();
    LoggerObjectTable(getDB()).add(log, true);
    getDB()->close();
    if (log.getPriority() > LogFile::INFO) {
      NSMCommunicator& com(*getCommunicator());
      com.sendLog(log);
      if (m_callback && m_callback->getCommunicator()) {
        NSMCommunicator& com_g(*m_callback->getCommunicator());
        com_g.sendLog(log);
      }
    }
  }
  return true;
}

RunControlCallback::NSMNodeIterator
RunControlCallback::find(const std::string& nodename) throw()
{
  NSMNodeIterator it = m_node_v.begin();
  for (; it != m_node_v.end(); it++) {
    if (it->getName() == nodename) return it;
  }
  return m_node_v.end();
}

//RunControlCallback::NSMNodeIterator
bool RunControlCallback::synchronize(NSMNode& node) throw()
{
  RCState state = node.getState();
  if (!state.isStable()) return false;
  NSMNodeIterator it = m_node_v.begin();
  for (; it != m_node_v.end(); it++) {
    if (state != it->getState()) {
      return false;
    }
  }
  return true;
}

void RunControlCallback::prepareRun(NSMMessage& msg) throw()
{
  getDB()->connect();
  // load from runnumberDB table
  size_t expno = (msg.getNParams() > 0) ? msg.getParam(0) : 0;
  size_t runno = (msg.getNParams() > 1) ? msg.getParam(1) : 0;
  size_t subno = (msg.getNParams() > 2) ? msg.getParam(2) : 0;
  RunNumberInfoTable table(getDB());
  m_info = table.add(expno, runno, subno);
  msg.setNParams(3);
  msg.setParam(0, m_info.getExpNumber());
  msg.setParam(1, m_info.getRunNumber());
  msg.setParam(2, m_info.getSubNumber());
  rc_status* status = (rc_status*)m_data.get();
  status->expno = m_info.getExpNumber();
  status->runno = m_info.getRunNumber();
  status->subno = m_info.getSubNumber();
  status->stime = m_info.getRecordTime();
  m_setting.setRunNumber(m_info);
  if (msg.getLength() > 0) {
    StringList str_v = StringUtil::split(msg.getData(), '\n', 2);
    m_setting.setOperators(str_v[0]);
    m_setting.setComment(str_v[1]);
  }
  m_setting.setRunControl(getConfig().getObject());
  LoggerObjectTable(getDB()).add(m_setting);
  getDB()->close();
}

void RunControlCallback::postRun(NSMMessage&) throw()
{
  try {
    getDB()->connect();
    RunNumberInfoTable(getDB()).add(m_info);
    LoggerObjectTable ltable(getDB());
    for (size_t i = 0; i < m_data_v.size(); i++) {
      ltable.add(m_data_v[i], true);
    }
    RunSummary summary(getNode());
    summary.setState(getNode());
    summary.setCause(RunSummary::MANUAL);
    summary.setRunSetting(m_setting);
    summary.setNodeState(m_node_v);
    summary.setNodeData(m_data_v);
    rc_status* status = (rc_status*)m_data.get();
    status->stime = 0;
    ltable.add(summary, true);
    getDB()->close();
  } catch (const std::exception& e) {
    LogFile::error(e.what());
  }
}
bool RunControlCallback::send(NSMMessage msg) throw()
{
  NSMCommunicator& com(*getCommunicator());
  RCCommand cmd = msg.getRequestName();
  msg.setNodeName(m_node_v[0]);
  if (cmd == RCCommand::START) {
    prepareRun(msg);
  } else if (cmd == RCCommand::STOP || cmd == RCCommand::ABORT) {
    postRun(msg);
  }
  const size_t nobj = getConfig().getObject().getNObjects("node");
  for (size_t i = 0; i < nobj; i++) {
    if (msg.getNodeName() == m_node_v[i].getName()) {
      if (cmd == RCCommand::LOAD) {
        const DBObject& obj(getConfig().getObject().getObject("node", i));
        const DBObject& cobj(obj.getObject("runtype"));
        if (cobj.getTable() == "ttd") {
          int pars[4];
          pars[0] = cobj.getShort("trigger_type");
          pars[1] = cobj.getShort("dummy_rate");
          pars[2] = cobj.getShort("trigger_limit");
          pars[3] = 0;
          com.sendRequest(NSMMessage(m_node_v[i], RCCommand::TRIGFT, 4, pars));
        }
        if (m_port > 0) {
          msg.setNParams(4);
          msg.setParam(0, NSMCommand::DBGET.getId());
          msg.setParam(1, cobj.getId());
          msg.setParam(2, m_port);
          msg.setParam(3, i);
        } else {
          msg.setNParams(2);
          msg.setParam(0, NSMCommand::DBGET.getId());
          msg.setParam(1, cobj.getId());
        }
      }
      if (cmd.isAvailable(m_node_v[i].getState())) {
        LogFile::debug("%s>>%s", cmd.getLabel(), m_node_v[i].getName().c_str());
        com.sendRequest(msg);
        RCState tstate = cmd.nextTState();
        if (tstate != Enum::UNKNOWN) m_node_v[i].setState(tstate);
      }
      if (i < nobj - 1) {
        msg.setNodeName(m_node_v[i + 1]);
        usleep(10000);
      }
    }
  }
  return true;
}

bool RunControlCallback::pause() throw()
{
  NSMMessage& msg(getMessage());
  RCState state(msg.getData());
  NSMNodeIterator it = find(msg.getNodeName());
  if (it != m_node_v.end()) {
    NSMNode& node(*it);
    node.setState(state);
    getNode().setState(state);
    DAQLogMessage log(msg.getNodeName(), LogFile::WARNING,
                      "Reuested PAUSE", Date());
    getDB()->connect();
    LoggerObjectTable(getDB()).add(log, true);
    getDB()->close();
  } else {
    getNode().setState(RCState::PAUSED_S);
  }
  const size_t nobj = getConfig().getObject().getNObjects("node");
  for (size_t i = 0; i < nobj; i++) {
    const DBObject& obj(getConfig().getObject().getObject("node", i));
    const DBObject& cobj(obj.getObject("runtype"));
    if (cobj.getTable() == "ttd") {
      try {
        sendPause(m_node_v[i]);
        return true;
      } catch (const IOException& e) {
        LogFile::error(e.what());
      }
      return false;
    }
  }
  try {
    if (m_callback && m_callback->getCommunicator()) {
      m_callback->sendPause();
      return true;
    }
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return false;
}

void RunControlCallback::ConfigProvider::run()
{
  TCPServerSocket server(m_hostname, m_port);
  try {
    server.open();
  } catch (const IOException& e) {
    LogFile::error("failed to open server socket (%s:%d)",
                   m_hostname.c_str(), m_port);
    exit(1);
  }
  while (true) {
    TCPSocket socket;
    try {
      socket = server.accept();
      TCPSocketReader reader(socket);
      size_t i = reader.readInt();
      const size_t nobj = m_callback->getConfig().getObject().getNObjects("node");
      if (i < nobj) {
        const DBObject& obj(m_callback->getConfig().getObject().getObject("node", i));
        const DBObject& cobj(obj.getObject("runtype"));
        TCPSocketWriter writer(socket);
        writer.writeObject(cobj);
      }
    } catch (const IOException& e) {
      LogFile::error(e.what());
    }
    socket.close();
  }
}

