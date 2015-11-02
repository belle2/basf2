#include "daq/slc/hvcontrol/HVMasterCallback.h"

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/DBObjectLoader.h>
#include <daq/slc/database/DAQLogDB.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/Date.h>

#include <algorithm>

using namespace Belle2;

HVMasterCallback::HVMasterCallback()
{
  reg(HVCommand::CONFIGURE);
  reg(HVCommand::STANDBY);
  reg(HVCommand::SHOULDER);
  reg(HVCommand::PEAK);
  reg(HVCommand::TURNON);
  reg(HVCommand::TURNOFF);
  m_state_demand = HVState::OFF_S;
  //setAutoReply(false);
  //m_showall = false;
}

bool HVMasterCallback::perform(NSMCommunicator& com) throw()
{
  NSMMessage msg(com.getMessage());
  if (NSMCallback::perform(com)) return true;
  try {
    HVCommand cmd(msg.getRequestName());
    if (cmd == HVCommand::UNKNOWN) return false;
    HVState state(getNode().getState());
    addNode(NSMNode(msg.getNodeName()));
    if (cmd == HVCommand::TURNOFF) {
      getNode().setState(HVState::TRANSITION_TS);
      turnoff();
    } else if (state.isOff()) {
      if (cmd == HVCommand::CONFIGURE) {
        const char* data = msg.getData();
        /*
              if (msg.getLength() && strlen(data) > 0) {
                m_confignames = data;
              } else {
                LogFile::warning("No config name");
              }
              LogFile::debug("config name : %s", m_confignames.c_str());
              dbload(m_confignames);
              configure(getConfig());
        */
      } else if (cmd == HVCommand::TURNON) {
        getNode().setState(HVState::TRANSITION_TS);
        m_state_demand = HVState::STANDBY_S;
        turnon();
      }
    } else if (state.isOn()) {
      if (cmd == HVCommand::STANDBY) {
        //if (state != HVState::STANDBY_S) {
        getNode().setState(HVState::TRANSITION_TS);
        m_state_demand = HVState::STANDBY_S;
        standby();
        //}
      } else if (cmd == HVCommand::SHOULDER) {
        //if (state != HVState::SHOULDER_S) {
        getNode().setState(HVState::TRANSITION_TS);
        m_state_demand = HVState::SHOULDER_S;
        shoulder();
        //}
      } else if (cmd == HVCommand::PEAK) {
        //if (state != HVState::PEAK_S) {
        getNode().setState(HVState::TRANSITION_TS);
        m_state_demand = HVState::PEAK_S;
        peak();
        //}
      }
    }
  } catch (const HVHandlerException& e) {
    reply(NSMMessage(NSMCommand::ERROR, e.what()));
  }
  set("state", getNode().getState().getLabel());
  return true;
}

void HVMasterCallback::turnon() throw(HVHandlerException)
{
  distribute(NSMMessage(HVCommand::TURNON));
}

void HVMasterCallback::turnoff() throw(HVHandlerException)
{
  distribute(NSMMessage(HVCommand::TURNOFF));
}

void HVMasterCallback::standby() throw(HVHandlerException)
{
  distribute(NSMMessage(HVCommand::STANDBY));
}

void HVMasterCallback::shoulder() throw(HVHandlerException)
{
  distribute(NSMMessage(HVCommand::SHOULDER));
}

void HVMasterCallback::peak() throw(HVHandlerException)
{
  distribute(NSMMessage(HVCommand::PEAK));
}

void HVMasterCallback::recover() throw(HVHandlerException)
{

}

void HVMasterCallback::initialize(const DBObject& obj) throw(HVHandlerException)
{
  //DBInterface& db(*getDB());
  if (!addAll(obj)) {
    throw (HVHandlerException("Failed to initialize (config=%s)", obj.getName().c_str()));
  }
}

void HVMasterCallback::configure(const DBObject& obj) throw(HVHandlerException)
{
  if (addAll(obj)) {
    distribute(NSMMessage(HVCommand::CONFIGURE));
  } else {
    throw (HVHandlerException("Failed to configure (config=%s)", obj.getName().c_str()));
  }
}

void HVMasterCallback::setState(const HVState& state) throw()
{
  if (getNode().getState() != state) {
    LogFile::debug("state transit : %s >> %s",
                   getNode().getState().getLabel(), state.getLabel());
    getNode().setState(state);
    try {
      set("rcstate", state.getLabel());
    } catch (const std::exception& e) {
      LogFile::error(e.what());
    }
  }
  reply(NSMMessage(NSMCommand::OK, state.getLabel()));
}

void HVMasterCallback::setState(NSMNode& node, const HVState& state) throw()
{
  node.setState(state);
  std::string vname = StringUtil::tolower(node.getName()) + ".rcstate";
  set(vname, state.getLabel());
}

void HVMasterCallback::ok(const char* nodename, const char* data) throw()
{
  LogFile::debug("OK from %s (state = %s)", nodename, data);
  HVState state(data);
  try {
    NSMNode& node(findNode(nodename));
    if (node.isUsed()) {
      if (state == NSMState::UNKNOWN) {
        LogFile::warning("got unknown state (%s) from %s", data, nodename);
      } else {
        setState(node, state);
      }
    }
  } catch (const std::out_of_range& e) {}
  //monitor();
}

void HVMasterCallback::error(const char* nodename, const char* data) throw()
{
  try {
    NSMNode& node(findNode(nodename));
    logging(node, LogFile::ERROR, data);
  } catch (const std::out_of_range& e) {
    LogFile::warning("ERROR from unknown node %s : %s", nodename, data);
  }
  //monitor();
}

void HVMasterCallback::fatal(const char* nodename, const char* data) throw()
{
  try {
    NSMNode& node(findNode(nodename));
    logging(node, LogFile::FATAL, data);
    //setState(RCState::ABORTING_RS);
    //abort();
  } catch (const std::out_of_range& e) {
    LogFile::warning("ERROR from unknown node %s : %s", nodename, data);
  }
  //monitor();
}

void HVMasterCallback::distribute(NSMMessage msg) throw()
{
  std::for_each(m_node_v.begin(), m_node_v.end(), Distributor(*this, msg));
}

void HVMasterCallback::distribute_r(NSMMessage msg) throw()
{
  std::for_each(m_node_v.rbegin(), m_node_v.rend(), Distributor(*this, msg));
}

NSMNode& HVMasterCallback::findNode(const std::string& name) throw(std::out_of_range)
{
  for (NSMNodeIterator it = m_node_v.begin(); it != m_node_v.end(); it++) {
    if (it->getName() == name) return *it;
  }
  throw (std::out_of_range(StringUtil::form("no node %s was found", name.c_str())));
}

bool HVMasterCallback::check(const std::string& node, const HVState& state) throw()
{
  for (NSMNodeIterator it = m_node_v.begin(); it != m_node_v.end(); it++) {
    if (it->getName() == node) return true;
    if (it->isUsed() && it->getState() != state) {
      return false;
    }
  }
  return true;
}

void HVMasterCallback::logging(const NSMNode& node, LogFile::Priority pri, const char* text, ...)
{
  static char buf[1024 * 100];
  va_list ap;
  va_start(ap, text);
  vsprintf(buf, text, ap);
  va_end(ap);
  logging_imp(node, pri, Date(), buf, false);
}

void HVMasterCallback::logging_imp(const NSMNode& node, LogFile::Priority pri,
                                   const Date& date, const std::string& msg,
                                   bool recorded)
{
  DAQLogMessage log(node.getName(), pri, msg, date);
  LogFile::put(pri, msg);
  DBInterface& db(*getDB());
  try {
    if (!recorded) {
      db.connect();
      DAQLogDB::createLog(db, m_logtable, log);
    }
  } catch (const DBHandlerException& e) {
    db.close();
    LogFile::error("DB errir : %s", e.what());
  }
  //reply(NSMMessage(log, true));
}

bool HVMasterCallback::addAll(const DBObject& obj) throw()
{
  NSMNodeList node_v;
  try {
    obj.print();
    const DBObjectList& objs(obj.getObjects("node"));
    for (DBObjectList::const_iterator it = objs.begin(); it != objs.end(); it++) {
      const DBObject& o_node(*it);
      NSMNode node(o_node.getText("name"));
      try {
        NSMNode& node_i(findNode(node.getName()));
        node = node_i;
      } catch (const std::out_of_range& e) {
        LogFile::info("New node : " + node.getName());
      }
      //node.setUsed(o_node.getBool("used"));
      if (o_node.hasObject("config")) {
        const std::string path = o_node("config").getPath();
        LogFile::info("found rcconfig :%s %s", node.getName().c_str(), path.c_str());
        //node.setConfig(path);
      } else {
        LogFile::warning("Not found rcconfig");
      }
      node_v.push_back(node);
    }
  } catch (const DBHandlerException& e) {
    LogFile::error("Failed to load db : %s", e.what());
    return false;
  }
  m_node_v = node_v;
  add(new NSMVHandlerInt("nnodes", true, false, (int)m_node_v.size()));
  for (size_t i = 0; i < m_node_v.size(); i++) {
    NSMNode& node(m_node_v[i]);
    std::string vname = StringUtil::form("node[%d]", (int)i);
    add(new NSMVHandlerText(vname + ".name", true, false, node.getName()));
    std::string table = "";
    try {
      get(node, "dbtable", table, 1);
    } catch (const TimeoutException& e) {}
    /*
    add(new NSMVHandlerHVConfig(*this, vname + ".config", node));
    add(new NSMVHandlerHVState(*this, vname + ".state", node));
    add(new NSMVHandlerHVRequest(*this, vname + ".request", node));
    add(new NSMVHandlerHVNodeUsed(*this, vname + ".used", node));
    vname = StringUtil::form("%s", StringUtil::tolower(node.getName()).c_str());
    add(new NSMVHandlerText(vname + ".dbtable", true, false, table));
    add(new NSMVHandlerHVConfig(*this, vname + ".config", node));
    add(new NSMVHandlerHVState(*this, vname + ".state", node));
    add(new NSMVHandlerHVRequest(*this, vname + ".request", node));
    add(new NSMVHandlerHVNodeUsed(*this, vname + ".used", node));
    */
  }
  add(new NSMVHandlerText("log.dbtable",  true, false, m_logtable));
  return true;
}

void HVMasterCallback::Distributor::operator()(NSMNode& node) throw()
{
  if (!m_enabled) return;
  m_msg.setNodeName(node);
  HVCommand cmd(m_msg.getRequestName());
  if (m_msg.getNodeName() == node.getName()) {
    if (node.isUsed()) {
      //if (cmd == HVCommand::CONFIGURE)
      //m_msg.setData(node.getConfig());
      try {
        if (NSMCommunicator::send(m_msg)) {
          //HVState tstate = HVCommand(m_msg.getRequestName()).nextTState();
          //if (tstate != Enum::UNKNOWN)
          //  m_callback.setState(node, tstate);
        } else {
          if (node.getState() != HVState::UNKNOWN) {
            m_callback.logging(m_callback.getNode(), LogFile::ERROR,
                               "%s is down.", node.getName().c_str());
          }
          m_callback.setState(node, HVState::UNKNOWN);
          m_callback.setState(HVState::ERROR_ES);
        }
      } catch (const NSMHandlerException& e) {
        LogFile::fatal("Failed to NSM2 request");
        m_enabled = false;
      } catch (const IOException& e) {
        LogFile::error(e.what());
        m_enabled = false;
        m_callback.setState(HVState::UNKNOWN);
      }
    }
  }
}

