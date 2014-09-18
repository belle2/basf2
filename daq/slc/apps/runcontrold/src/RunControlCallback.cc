#include "daq/slc/apps/runcontrold/RunControlCallback.h"

#include <daq/slc/apps/runcontrold/RunSetting.h>
#include <daq/slc/apps/runcontrold/RunSummary.h>
#include <daq/slc/apps/runcontrold/rc_status.h>
#include <daq/slc/apps/runcontrold/rorc_status.h>

#include <daq/slc/readout/ronode_status.h>

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/LoggerObjectTable.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>
#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/TCPSocketWriter.h>

#include <daq/slc/base/ConfigFile.h>

#include <cstring>
#include <cstdlib>
#include <sstream>
#include <unistd.h>

using namespace Belle2;

RunControlCallback::RunControlCallback(const NSMNode& node,
                                       const std::string& runtype,
                                       const std::string& format,
                                       int revision, int port)
  : RCCallback(node), m_setting(node)
{
  m_data = NSMData(node.getName() + "_STATUS", format, revision);
  m_runtype_default = runtype;
  m_port = port;
  m_callback = NULL;
  setAutoReply(false);
  setDBClose(false);
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
      NSMNode node(obj.getNode());
      node.setUsed(obj_v[i].getBool("used"));
      for (StringList::iterator it = m_excluded_v.begin();
           it != m_excluded_v.end(); it++) {
        if (node.getName() == *it) {
          node.setExcluded(true);
        }
      }
      m_node_v.push_back(node);
    }
  }
  getNode().setState(Enum::UNKNOWN);
  timeout();
  if (m_port > 0) {
    PThread(new ConfigProvider(this, "0.0.0.0", m_port));
  }
  logging(DAQLogMessage(getNode().getName(), LogFile::INFO,
                        "started runcontrol:" + getNode().getName()));
}

bool RunControlCallback::sendState(const NSMNode& node) throw()
{
  NSMCommunicator& com(*getCommunicator());
  com.sendState(node);
  if (m_callback && m_callback->getCommunicator()) {
    NSMCommunicator& com_g(*m_callback->getCommunicator());
    com_g.sendState(node);
  }
  return true;
}

bool RunControlCallback::replyOK() throw()
{
  NSMCommunicator& com(*getCommunicator());
  com.replyOK(getNode());
  if (m_callback && m_callback->getCommunicator()) {
    NSMCommunicator& com_g(*m_callback->getCommunicator());
    com_g.replyOK(getNode());
  }
  return true;
}

bool RunControlCallback::ok() throw()
{
  NSMMessage& msg(getMessage());
  LogFile::debug("OK from %s (state = %s)",
                 msg.getNodeName(), msg.getData());
  if (msg.getLength() > 0) {
    RCState state(msg.getData());
    const std::string nodename = msg.getNodeName();
    NSMNodeIterator it = findNode(nodename);
    if (state == NSMState::UNKNOWN) {
      LogFile::error("got unknown state (%s) from %s",
                     msg.getData(), nodename.c_str());
    } else if (it != m_node_v.end()) {
      NSMNode& node(*it);
      node.setState(state);
      logging(DAQLogMessage(getNode().getName(), LogFile::INFO,
                            StringUtil::form("OK from %s (state = %s)",
                                             nodename.c_str(), state.getLabel())));
      sendState(node);
      if (state == RCState::PAUSED_S) {
        NSMCommunicator& com(*getCommunicator());
        const size_t nobj = getConfig().getObject().getNObjects("node");
        for (size_t i = 0; i < nobj; i++) {
          const DBObject& obj(getConfig().getObject().getObject("node", i));
          if (obj.getBool("used") && !m_node_v[i].isExcluded()) {
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
      }
      if (state.isStable() && synchronize(node)) {
        RCState state_org(getNode().getState());
        if (state != state_org) {
          getNode().setState(state);
          replyOK();
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
    logging(DAQLogMessage(nodename, LogFile::ERROR,
                          StringUtil::form("ERROR message : %s",
                                           msg.getData())));
  }
  return true;
}

RunControlCallback::NSMNodeIterator
RunControlCallback::findNode(const std::string& nodename) throw()
{
  for (NSMNodeIterator it = m_node_v.begin();
       it != m_node_v.end(); it++) {
    if (it->getName() == nodename) return it;
  }
  return m_node_v.end();
}

ConfigObjectList::iterator
RunControlCallback::findConfig(const std::string& nodename, bool& finded) throw()
{
  ConfigObjectList& objs(getConfig().getObject().getObjects("node"));
  for (ConfigObjectList::iterator it = objs.begin();
       it != objs.end(); it++) {
    if (it->getObject("runtype").getNode() == nodename) {
      finded = true;
      return it;
    }
  }
  finded = false;
  return objs.end();
}

bool RunControlCallback::synchronize(NSMNode& node) throw()
{
  RCState state = node.getState();
  if (!state.isStable()) return false;
  NSMNodeIterator it = m_node_v.begin();
  int i = 0;
  for (; it != m_node_v.end(); it++) {
    const DBObject& obj(getConfig().getObject().getObject("node", i));
    i++;
    if (obj.getBool("used") && !it->isExcluded() && state != it->getState()) {
      return false;
    }
  }
  return true;
}

void RunControlCallback::postRun() throw()
{
  try {
    if (!getDB()->isConnected())
      getDB()->connect();
    m_info.setId(0);
    RunNumberInfoTable(getDB()).add(m_info);
    LoggerObjectTable ltable(getDB());
    for (size_t i = 0; i < m_data_v.size(); i++) {
      m_data_v[i].setId(0);
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
    //getDB()->close();
  } catch (const std::exception& e) {
    //getDB()->close();
  }
}

bool RunControlCallback::load() throw()
{
  NSMCommunicator& com(*getCommunicator());
  NSMMessage& msg(getMessage());
  for (size_t i = 0; i < m_node_v.size(); i++) {
    NSMNode& node(m_node_v[i]);
    msg.setNodeName(node);
    bool finded;
    ConfigObjectList::iterator it = findConfig(node.getName(), finded);
    if (!finded) continue;
    node.setUsed(it->getBool("used"));
    if (node.isUsed() && !node.isExcluded() &&
        msg.getNodeName() == node.getName()) {
      const DBObject& cobj(it->getObject("runtype"));
      if (cobj.getTable() == "ttd") {
        int pars[4];
        pars[0] = cobj.getShort("trigger_type");
        pars[1] = cobj.getShort("dummy_rate");
        pars[2] = cobj.getShort("trigger_limit");
        pars[3] = 0;
        LogFile::debug("Send TRIGFT to %s (pars = {%d, %d, %d})",
                       node.getName().c_str(),
                       pars[0], pars[1], pars[2]);
        com.sendRequest(NSMMessage(node, RCCommand::TRIGFT, 4, pars));
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
      LogFile::debug("Send %s to %s", msg.getRequestName(),
                     node.getName().c_str());
      logging(DAQLogMessage(getNode().getName(), LogFile::DEBUG,
                            StringUtil::form("Send %s to %s",
                                             msg.getRequestName(),
                                             node.getName().c_str())));
      com.sendRequest(msg);
      node.setState(RCState::LOADING_TS);
    }
  }
  return true;
}

bool RunControlCallback::distribute(NSMMessage& msg) throw()
{
  NSMCommunicator& com(*getCommunicator());
  for (size_t i = 0; i < m_node_v.size(); i++) {
    NSMNode& node(m_node_v[i]);
    msg.setNodeName(node);
    if (node.isUsed() && !node.isExcluded() &&
        msg.getNodeName() == node.getName()) {
      LogFile::debug("Send %s to %s", msg.getRequestName(),
                     node.getName().c_str());
      com.sendRequest(msg);
      RCState tstate = RCCommand(msg.getRequestName()).nextTState();
      if (tstate != Enum::UNKNOWN) node.setState(tstate);
    }
  }
  return true;
}

bool RunControlCallback::distribute_r(NSMMessage& msg) throw()
{
  NSMCommunicator& com(*getCommunicator());
  for (size_t i = m_node_v.size() - 1; true; i--) {
    NSMNode& node(m_node_v[i]);
    msg.setNodeName(node);
    if (node.isUsed() && !node.isExcluded() &&
        msg.getNodeName() == node.getName()) {
      LogFile::debug("Send %s to %s", msg.getRequestName(),
                     node.getName().c_str());
      com.sendRequest(msg);
      RCState tstate = RCCommand(msg.getRequestName()).nextTState();
      if (tstate != Enum::UNKNOWN) node.setState(tstate);
    }
    if (i == 0) break;
  }
  return true;
}

bool RunControlCallback::start() throw()
{
  NSMMessage& msg(getMessage());
  if (!getDB()->isConnected())
    getDB()->connect();
  size_t expno = (msg.getNParams() > 0) ? msg.getParam(0) : 0;
  size_t runno = (msg.getNParams() > 1) ? msg.getParam(1) : 0;
  size_t subno = (msg.getNParams() > 2) ? msg.getParam(2) : 0;
  size_t stime = (msg.getNParams() > 3) ? msg.getParam(3) : 0;
  if (msg.getNParams() < 2) {
    RunNumberInfoTable table(getDB());
    m_info = table.add(expno, runno, subno);
    msg.setNParams(4);
    msg.setParam(0, m_info.getExpNumber());
    msg.setParam(1, m_info.getRunNumber());
    msg.setParam(2, m_info.getSubNumber());
    msg.setParam(3, m_info.getRecordTime());
    expno = m_info.getExpNumber();
    runno = m_info.getRunNumber();
    subno = m_info.getSubNumber();
    stime = m_info.getRecordTime();
    m_setting.setRunNumber(m_info);
  }
  if (msg.getLength() > 0) {
    StringList str_v = StringUtil::split(msg.getData(), '\n', 2);
    m_setting.setOperators(str_v[0]);
    m_setting.setComment(str_v[1]);
  }
  rc_status* status = (rc_status*)m_data.get();
  status->expno = expno;
  status->runno = runno;
  status->subno = subno;
  status->stime = stime;
  status->configid = getConfig().getObject().getId();
  m_setting.setRunControl(getConfig().getObject());
  LoggerObjectTable(getDB()).add(m_setting);
  //getDB()->close();
  update();
  return distribute(msg);
}

bool RunControlCallback::stop() throw()
{
  postRun();
  return distribute(getMessage());
}

bool RunControlCallback::abort() throw()
{
  postRun();
  return distribute_r(getMessage());
}

bool RunControlCallback::pause() throw()
{
  NSMMessage& msg(getMessage());
  RCState state(msg.getData());
  NSMNodeIterator it = findNode(msg.getNodeName());
  if (it != m_node_v.end()) {
    NSMNode& node(*it);
    node.setState(state);
    getNode().setState(state);
    DAQLogMessage log(msg.getNodeName(), LogFile::WARNING,
                      "Reuested PAUSE", Date());
    if (!getDB()->isConnected())
      getDB()->connect();
    LoggerObjectTable(getDB()).add(log, true);
    //getDB()->close();
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
    logging(log, msg.getParam(2) > 0);
  }
  return true;
}

void RunControlCallback::logging(const DAQLogMessage& log, bool recoreded)
{
  try {
    if (log.getPriority() >= getPriorityToDB() && !recoreded) {
      if (!getDB()->isConnected())
        getDB()->connect();
      LoggerObjectTable(getDB()).add(log, true);
      //getDB()->close();
    }
    if (log.getPriority() >= getPriorityToLocal()) {
      NSMCommunicator* com = getCommunicator();
      com->sendLog(log, true);
      if (m_callback && m_callback->getCommunicator()
          && log.getPriority() >= getPriorityToGlobal()) {
        NSMCommunicator* com_g = m_callback->getCommunicator();
        com_g->sendLog(log, true);
      }
    }
  } catch (const DBHandlerException& e) {
    getDB()->close();
    LogFile::error("DB errir : %s", e.what());
  }
}

bool RunControlCallback::exclude() throw()
{
  const NSMMessage& msg(getMessage());
  NSMNodeIterator it = findNode(msg.getData());
  try {
    if (it != m_node_v.end()) {
      NSMNode& node(*it);
      node.setExcluded(true);
      try {
        LogFile::error("Excluded " + node.getName());
        if (!getDB()->isConnected())
          getDB()->connect();
        DAQLogMessage log(getNode().getName(), LogFile::INFO,
                          "Excluded " + node.getName());
        LoggerObjectTable(getDB()).add(log, true);
        //getDB()->close();
      } catch (const DBHandlerException& e) {
        getDB()->close();
        LogFile::error("DB errir : %s", e.what());
      }
    }
    return true;
  } catch (const NSMHandlerException& e) {

  }
  return false;
}

bool RunControlCallback::include() throw()
{
  const NSMMessage& msg(getMessage());
  NSMNodeIterator it = findNode(msg.getData());
  try {
    if (it != m_node_v.end()) {
      NSMNode& node(*it);
      node.setExcluded(false);
      try {
        LogFile::error("Included " + node.getName());
        if (!getDB()->isConnected())
          getDB()->connect();
        DAQLogMessage log(getNode().getName(), LogFile::INFO,
                          "Included " + node.getName());
        LoggerObjectTable(getDB()).add(log, true);
        //getDB()->close();
      } catch (const DBHandlerException& e) {
        getDB()->close();
        LogFile::error("DB errir : %s", e.what());
      }
    }
    return true;
  } catch (const NSMHandlerException& e) {

  }
  return false;
}

void RunControlCallback::timeout() throw()
{
  static unsigned long long count = 0;
  NSMCommunicator& com(*getCommunicator());
  for (size_t i = 0; i < m_node_v.size(); i++) {
    NSMNode& node(m_node_v[i]);
    RCState state(node.getState());
    if (!com.isConnected(node)) {
      if (node.getState() != Enum::UNKNOWN) {
        logging(DAQLogMessage(getNode().getName(), LogFile::WARNING,
                              node.getName() + " got down."));
        node.setState(Enum::UNKNOWN);
      }
      continue;
    }
    if (!state.isStable() && count % 2 == 0) {
      try {
        LogFile::debug("STATECHECK >> %s", node.getName().c_str());
        com.sendRequest(NSMMessage(node, RCCommand::STATECHECK));
      } catch (const NSMHandlerException& e) {
        logging(DAQLogMessage(getNode().getName(), LogFile::WARNING,
                              "Failed to send statecheck : " +
                              node.getName()));
        LogFile::warning("Failed to send statecheck : %s",
                         node.getName().c_str());
      }
    }
  }
  if (count % 5 == 0) {
    try {
      if (!getDB()->isConnected())
        getDB()->connect();
      LoggerObjectTable ltable(getDB());
      for (size_t i = 0; i < m_data_v.size(); i++) {
        if (m_data_v[i].isAvailable()) {
          m_data_v[i].setId(0);
          ltable.add(m_data_v[i], true);
        }
      }
      //getDB()->close();
    } catch (const std::exception& e) {
      LogFile::error(e.what());
      getDB()->close();
    }
  }
  count++;
  update();
}

void RunControlCallback::update() throw()
{
  ConfigObjectList& obj_v(getConfig().getObject().getObjects("node"));
  for (size_t i = 0; i < m_data_v.size(); i++) {
    NSMData& data(m_data_v[i]);
    if (!data.isAvailable()) {
      try {
        data.open(getCommunicator());
        LogFile::debug("Opened nsm data : %s ", data.getName().c_str());
        logging(DAQLogMessage(getNode().getName(), LogFile::INFO,
                              "Opened nsm data :  " + data.getName()));
      } catch (const NSMHandlerException& e) {
        LogFile::warning("Failed to open nsm data : %s", data.getName().c_str());
      }
    }
  }
  bool aborting_any = false;
  bool recovering_any = false;
  bool notready_any = false;
  bool notready_all = true;
  bool ready_all = true;
  bool running_all = true;
  bool starting_any = false;
  bool stopping_any = false;
  bool paused_any = false;
  int count = 0;
  for (size_t i = 0; i < m_node_v.size(); i++) {
    const DBObject& obj(getConfig().getObject().getObject("node", i));
    if (obj.getBool("used") && !m_node_v[i].isExcluded()) {
      count++;
      const RCState& state(m_node_v[i].getState());
      if (state != RCState::NOTREADY_S) {
        notready_all = false;
      }
      if (state == RCState::ABORTING_RS) {
        aborting_any = true;
      } else if (state == RCState::RECOVERING_RS) {
        recovering_any = true;
      } else if (state == RCState::UNKNOWN || state == RCState::NOTREADY_S ||
                 state == RCState::LOADING_TS) {
        notready_any = true;
        ready_all = false;
        running_all = false;
      } else if (state == RCState::STARTING_TS) {
        starting_any = true;
      } else if (state == RCState::STOPPING_TS) {
        stopping_any = true;
      } else if (state == RCState::PAUSED_S) {
        paused_any = true;
      } else if (state == RCState::RUNNING_S) {
        ready_all = false;
      }
    }
  }
  RCState state_org = getNode().getState();
  if (count == 0) {
    getNode().setState(RCState::NOTREADY_S);
  } else if (aborting_any ||
             (!notready_all && state_org == RCState::ABORTING_RS)) {
    getNode().setState(RCState::ABORTING_RS);
  } else if (recovering_any ||
             (!ready_all && state_org == RCState::RECOVERING_RS)) {
    getNode().setState(RCState::RECOVERING_RS);
  } else if (notready_any) {
    getNode().setState(RCState::NOTREADY_S);
  } else if (ready_all) {
    getNode().setState(RCState::READY_S);
  } else if (paused_any) {
    getNode().setState(RCState::PAUSED_S);
  } else if (starting_any ||
             (!running_all && state_org == RCState::STARTING_TS)) {
    getNode().setState(RCState::STARTING_TS);
  } else if (stopping_any ||
             (!ready_all && state_org == RCState::STARTING_TS)) {
    getNode().setState(RCState::STOPPING_TS);
  } else if (running_all) {
    getNode().setState(RCState::RUNNING_S);
  }
  if (getNode().getState() != state_org) {
    replyOK();
  }
  void* data = m_data.get();
  rc_status* status = (rc_status*)data;
  bool isro = m_data.getFormat().find("rorc_status") != std::string::npos;
  for (size_t i = 0; i < m_node_v.size(); i++) {
    status->node[i].eflag = m_node_v[i].getError();
    status->node[i].state = m_node_v[i].getState().getId();
    status->node[i].excluded = m_node_v[i].isExcluded();
    DBObject& obj(obj_v[i].getObject("runtype"));
    status->node[i].configid = obj.getId();
    if (isro && m_data_v[i].isAvailable() &&
        m_data_v[i].getFormat() == "ronode_status") {
      m_data_v[i].update();
      rorc_status::ro_status* rost = &(((rorc_status*)data)->ro[i]);
      ronode_status* ronode = (ronode_status*)m_data_v[i].get();
      rost->nevent_in = ronode->nevent_in;
      rost->nqueue_in = ronode->nqueue_in;
      rost->connection_in = ronode->connection_in;
      rost->nevent_out = ronode->nevent_out;
      rost->nqueue_out = ronode->nqueue_out;
      rost->connection_out = ronode->connection_out;
      rost->reserved_i[0] = ronode->reserved_i[0];
      rost->reserved_i[1] = ronode->reserved_i[1];
      rost->evtrate_in = ronode->evtrate_in;
      rost->evtsize_in = ronode->evtsize_in;
      rost->flowrate_in = ronode->flowrate_in;
      rost->evtrate_out = ronode->evtrate_out;
      rost->evtsize_out = ronode->evtsize_out;
      rost->flowrate_out = ronode->flowrate_out;
      rost->loadavg = ronode->loadavg;
      rost->reserved_f[0] = ronode->reserved_f[0];
      rost->reserved_f[1] = ronode->reserved_f[1];
      rost->reserved_f[2] = ronode->reserved_f[2];
      rost->reserved_f[3] = ronode->reserved_f[3];
      rost->reserved_f[4] = ronode->reserved_f[4];
    }
  }
  status->state = getNode().getState().getId();
  status->configid = getConfig().getObject().getId();
  status->nnodes = m_node_v.size();
  status->expno = m_info.getExpNumber();
  status->runno = m_info.getRunNumber();
  status->subno = m_info.getSubNumber();
  status->ctime = Time().getSecond();
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
