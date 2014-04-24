#include "daq/slc/runcontrol/RunControlMasterCallback.h"

#include <daq/slc/runcontrol/RunSetting.h>
#include <daq/slc/runcontrol/RunSummary.h>
#include <daq/slc/runcontrol/rc_status.h>

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/LoggerObjectTable.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>

#include <cstring>
#include <unistd.h>

using namespace Belle2;

RunControlMasterCallback::RunControlMasterCallback(const NSMNode& node)
  : RCCallback(node), m_setting(node)
{
  m_data = NSMData(node.getName() + "_STATUS", "rc_status", rc_status_revision);
}

void RunControlMasterCallback::init() throw()
{
  ConfigFile config("runcontrol");
  std::string runtype = config.get("runtype");
  NSMMessage msg(getNode(), NSMCommand::DBGET, runtype);
  msg.setNParams(1);
  msg.setParam(0, NSMCommand::DBGET.getId());
  msg.setData(runtype);
  preload(msg);
  {
    ConfigObjectList& obj_v(getConfig().getObject().getObjects("nsmdata"));
    for (size_t i = 0; i < obj_v.size(); i++) {
      ConfigObject& obj(obj_v[i]);
      NSMData data(obj.getText("dataname"), obj.getText("format"),
                   obj.getInt("revision"));
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
  timeout();
}

void RunControlMasterCallback::timeout() throw()
{
  rc_status* status = (rc_status*)m_data.get();
  status->state = getNode().getState().getId();
  status->configid = getConfig().getObject().getId();
  status->nnodes = m_node_v.size();
  ConfigObjectList& obj_v(getConfig().getObject().getObjects("node"));
  NSMCommunicator& com(*getCommunicator());
  for (size_t i = 0; i < m_node_v.size(); i++) {
    NSMNode& node(m_node_v[i]);
    if (com.getMaster().getName().size() > 0 &&
        !RCState(node.getState()).isStable()) {
      try {
        com.sendRequest(NSMMessage(node, RCCommand::STATECHECK));
      } catch (const NSMHandlerException& e) {
        LogFile::warning("Failed to send statecheck : %s",
                         node.getName().c_str());
      }
    }
    status->node[i].state = node.getState().getId();
    DBObject& obj(obj_v[i].getObject("runtype"));
    status->node[i].configid = obj.getId();
  }
  for (size_t i = 0; i < m_data_v.size(); i++) {
    NSMData& data(m_data_v[i]);
    if (!data.isAvailable()) {
      try {
        data.open(getCommunicator());
        LogFile::debug("Opened nsm data : %s ",
                       data.getName().c_str());
      } catch (const NSMHandlerException& e) {
        LogFile::warning("Failed to open nsm data : %s",
                         data.getName().c_str());
      }
    }
  }
}

bool RunControlMasterCallback::ok() throw()
{
  NSMMessage& msg(getMessage());
  if (msg.getLength() > 0) {
    RCState state(msg.getData());
    const std::string nodename = msg.getNodeName();
    LogFile::debug("From %s", nodename.c_str());
    NSMNodeIterator it = find(nodename);
    if (it != m_node_v.end()) {
      NSMCommunicator& com(*getCommunicator());
      NSMNode& node(*it);
      node.setState(state);
      com.sendState(node);
      NSMNodeIterator it = synchronize(node);
      if (state.isStable() && it == m_node_v.end()) {
        getNode().setState(state);
        timeout();
        com.replyOK(getNode());
      } else if (it != m_node_v.end()) {
        m_msg_tmp.setNodeName(*it);
        timeout();
        send(m_msg_tmp);
      }
    } else {
      LogFile::error("OK request from Unknown node: %s",
                     nodename.c_str());
    }
  }
  return true;
}

bool RunControlMasterCallback::error() throw()
{
  NSMMessage& msg(getMessage());
  if (msg.getLength() > 0) {
    LogFile::error("Error from '%s' (%s)",
                   msg.getNodeName(), msg.getData());
  }
  return true;//?
}

RunControlMasterCallback::NSMNodeIterator
RunControlMasterCallback::find(const std::string& nodename) throw()
{
  NSMNodeIterator it = m_node_v.begin();
  for (; it != m_node_v.end(); it++) {
    if (it->getName() == nodename) return it;
  }
  return m_node_v.end();
}

RunControlMasterCallback::NSMNodeIterator
RunControlMasterCallback::synchronize(NSMNode& node) throw()
{
  RCState state = node.getState();
  NSMNodeIterator it = m_node_v.begin();
  for (; it != m_node_v.end(); it++) {
    if (state != it->getState()) {
      if (state.isStable()) return it;
      break;
    }
    if (node.getName() == it->getName()) return ++it;
  }
  return m_node_v.end();
}

bool RunControlMasterCallback::send(NSMMessage msg) throw()
{
  NSMCommunicator& com(*getCommunicator());
  RCCommand cmd = msg.getRequestName();
  if (msg.getNodeName() == com.getMaster().getName()) {
    msg.setNodeName(m_node_v[0]);
    if (cmd == RCCommand::START) {
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
        StringList str_v = StringUtil::split(msg.getData(), ' ', 2);
        m_setting.setOperators(str_v[0]);
        m_setting.setComment(str_v[1]);
      }
      m_setting.setRunControl(getConfig().getObject());
      LoggerObjectTable(getDB()).add(m_setting);
      getDB()->close();
    } else if (cmd == RCCommand::STOP) {
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
    }
    m_msg_tmp = msg;
  }
  const size_t nobj = getConfig().getObject().getNObjects("node");
  for (size_t i = 0; i < nobj; i++) {
    const DBObject& obj(getConfig().getObject().getObject("node", i));
    if (msg.getNodeName() == m_node_v[i].getName()) {
      if (cmd == RCCommand::LOAD) {
        msg.setNParams(1);
        const DBObject& cobj(obj.getObject("runtype"));
        if (obj.getEnum("dbmode") == "get") {
          msg.setParam(0, NSMCommand::DBGET.getId());
          msg.setData(cobj.getName());
        } else if (obj.getEnum("dbmode") == "set") {
          msg.setParam(0, NSMCommand::DBSET.getId());
          msg.setData(cobj);
        }
      }
      RCState tstate = cmd.nextTState();
      if (tstate != Enum::UNKNOWN) m_node_v[i].setState(tstate);
      com.sendRequest(msg);
      if (i < nobj - 1 && getConfig().getObject().getObject("node", i).getBool("sequential")) {
        break;
      }
    }
  }
  return true;
}
