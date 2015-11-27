#include "daq/slc/apps/hvcontrold/HVMasterCallback.h"
#include "daq/slc/apps/hvcontrold/HVMasterHandler.h"

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
}

bool HVMasterCallback::perform(NSMCommunicator& com) throw()
{
  NSMMessage msg(com.getMessage());
  if (NSMCallback::perform(com)) return true;
  try {
    HVCommand cmd(msg.getRequestName());
    if (cmd == HVCommand::UNKNOWN) return false;
    const HVState state(getNode().getState());
    addNode(HVNode(msg.getNodeName()));
    HVState state_demand = HVState::UNKNOWN;
    if (cmd == HVCommand::RECOVER && state != HVState::OFF_S) {
      state_demand = HVState::OFF_S;
      recover();
    } else if (cmd == HVCommand::TURNOFF && state != HVState::OFF_S) {
      state_demand = HVState::OFF_S;
      turnoff();
    } else if (state.isOff()) {
      if (cmd == HVCommand::CONFIGURE) {
        state_demand = HVState::OFF_S;
        if (msg.getLength() > 0) {
          const char* data = msg.getData();
          printf("new config : %s", data);
        }
      } else if (cmd == HVCommand::TURNON) {
        state_demand = HVState::STANDBY_S;
        turnon();
      }
    } else if (state.isOn()) {
      if (cmd == HVCommand::STANDBY && state != HVState::STANDBY_S) {
        state_demand = HVState::STANDBY_S;
        standby();
      } else if (cmd == HVCommand::SHOULDER && state != HVState::SHOULDER_S) {
        state_demand = HVState::SHOULDER_S;
        shoulder();
      } else if (cmd == HVCommand::PEAK && state != HVState::PEAK_S) {
        state_demand = HVState::PEAK_S;
        peak();
      }
    }
    if (state_demand != HVState::UNKNOWN) {
      HVState tstate = cmd.nextTState(state);
      if (tstate != HVState::UNKNOWN)
        setState(tstate);
    }
  } catch (const HVHandlerException& e) {
    reply(NSMMessage(NSMCommand::ERROR, e.what()));
  }
  return true;
}

void HVMasterCallback::configure(const DBObject& obj) throw(HVHandlerException)
{
  if (addAll(obj)) {
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
      set("state", state.getLabel());
    } catch (const std::exception& e) {
      LogFile::error(e.what());
    }
  }
  reply(NSMMessage(NSMCommand::OK, state.getLabel()));
}

void HVMasterCallback::setState(HVNode& node, const HVState& state) throw()
{
  node.setState(state);
  std::string vname = StringUtil::tolower(node.getName()) + ".state";
  set(vname, state.getLabel());
}

void HVMasterCallback::setConfig(HVNode& node, const std::string& config) throw()
{
  node.setConfig(config);
  std::string vname = StringUtil::tolower(node.getName()) + ".config";
  set(vname, config);
}

void HVMasterCallback::ok(const char* nodename, const char* data) throw()
{
  LogFile::debug("OK from %s (state = %s)", nodename, data);
  HVState state(data);
  try {
    HVNode& node(findNode(nodename));
    if (node.isUsed()) {
      if (state == NSMState::UNKNOWN) {
        LogFile::warning("got unknown state (%s) from %s", data, nodename);
      } else {
        setState(node, state);
      }
    }
  } catch (const std::out_of_range& e) {}
  monitor();
}

void HVMasterCallback::error(const char* nodename, const char* data) throw()
{
  try {
    HVNode& node(findNode(nodename));
    logging(node, LogFile::ERROR, data);
  } catch (const std::out_of_range& e) {
    LogFile::warning("ERROR from unknown node %s : %s", nodename, data);
  }
  monitor();
}

void HVMasterCallback::distribute(NSMMessage msg) throw()
{
  std::for_each(m_node_v.begin(), m_node_v.end(), Distributor(*this, msg));
}

void HVMasterCallback::distribute_r(NSMMessage msg) throw()
{
  std::for_each(m_node_v.rbegin(), m_node_v.rend(), Distributor(*this, msg));
}

HVNode& HVMasterCallback::findNode(const std::string& name) throw(std::out_of_range)
{
  for (HVNodeIterator it = m_node_v.begin(); it != m_node_v.end(); it++) {
    if (it->getName() == name) return *it;
  }
  throw (std::out_of_range(StringUtil::form("no node %s was found", name.c_str())));
}

bool HVMasterCallback::check(const std::string& node, const HVState& state) throw()
{
  for (HVNodeIterator it = m_node_v.begin(); it != m_node_v.end(); it++) {
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
  if (getDB() == NULL || m_logtable.size() == 0) return;
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

void HVMasterCallback::init(NSMCommunicator&) throw()
{
  NSMNode& node(getNode());
  node.setState(HVState::UNKNOWN);
  add(new NSMVHandlerText("dbtable", true, false, m_table));
  add(new NSMVHandlerText("state", true, false, node.getState().getLabel()));
  add(new NSMVHandlerText("request", true, false, ""));
  if (m_file.size() == 0 && (m_table.size() == 0 || m_config.size() == 0)) {
    LogFile::notice("dbtable or config is empty (dbtable='%s', runtype='%s')",
                    m_table.c_str(), m_config.c_str());
  } else {
    const std::string config = node.getName() + "@HV:" + m_config;
    if (m_file.size() > 0) {
      m_obj = DBObjectLoader::load(m_file);
      m_obj.print(false);
    } else if (getDB()) {
      DBInterface& db(*getDB());
      m_obj = DBObjectLoader::load(db, m_table, config, false);
      db.close();
      m_obj.print(false);
    }
  }
  add(new NSMVHandlerText("config", true, false, m_obj.getName()));
  add(m_obj);
  if (!addAll(m_obj)) {
    LogFile::fatal("Failed to initialize. terminating process");
    term();
    exit(1);
  }
  monitor();
}

bool HVMasterCallback::addAll(const DBObject& obj) throw()
{
  HVNodeList node_v;
  try {
    obj.print();
    const DBObjectList& objs(obj.getObjects("node"));
    for (DBObjectList::const_iterator it = objs.begin(); it != objs.end(); it++) {
      const DBObject& o_node(*it);
      HVNode node(o_node.getText("name"));
      try {
        HVNode& node_i(findNode(node.getName()));
        node = node_i;
      } catch (const std::out_of_range& e) {
        LogFile::info("New node : " + node.getName());
      }
      node.setUsed(o_node.getBool("used"));
      if (o_node.hasObject("config")) {
        const std::string path = o_node("config").getPath();
        LogFile::info("found config :%s %s", node.getName().c_str(), path.c_str());
        node.setConfig(path);
      } else {
        LogFile::warning("Not found config");
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
    HVNode& node(m_node_v[i]);
    std::string vname = StringUtil::form("node[%d]", (int)i);
    add(new NSMVHandlerText(vname + ".name", true, false, node.getName()));
    std::string table = "";
    try {
      get(node, "dbtable", table, 1);
    } catch (const TimeoutException& e) {}
    add(new NSMVHandlerHVState(*this, vname + ".state", node));
    add(new NSMVHandlerHVRequest(*this, vname + ".request", node));
    add(new NSMVHandlerHVNodeUsed(*this, vname + ".used", node));
    vname = StringUtil::form("%s", StringUtil::tolower(node.getName()).c_str());
    add(new NSMVHandlerText(vname + ".dbtable", true, false, table));
    add(new NSMVHandlerHVState(*this, vname + ".state", node));
    add(new NSMVHandlerHVRequest(*this, vname + ".request", node));
    add(new NSMVHandlerHVNodeUsed(*this, vname + ".used", node));
  }
  add(new NSMVHandlerText("log.dbtable",  true, false, m_logtable));
  return true;
}

void HVMasterCallback::Distributor::operator()(HVNode& node) throw()
{
  if (!m_enabled) return;
  m_msg.setNodeName(node);
  HVCommand cmd(m_msg.getRequestName());
  if (m_msg.getNodeName() == node.getName()) {
    if (node.isUsed()) {
      try {
        if (NSMCommunicator::send(m_msg)) {
          HVState tstate = cmd.nextTState(HVState(node.getState()));
          if (tstate != Enum::UNKNOWN)
            m_callback.setState(node, tstate);
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

void HVMasterCallback::monitor() throw(HVHandlerException)
{
  HVState state(getNode().getState());
  HVState state_new = HVState::UNKNOWN;
  bool synchronized = true;
  for (HVNodeIterator it = m_node_v.begin(); it != m_node_v.end(); it++) {
    HVNode& node(*it);
    if (!node.isUsed()) continue;
    HVState cstate(node.getState());
    try {
      NSMCommunicator::connected(node.getName());
      if (cstate == Enum::UNKNOWN) {
        try {
          std::string s;
          get(node, "state", s, 1);
          if ((cstate = HVState(s)) != Enum::UNKNOWN) {
            logging(getNode(), LogFile::NOTICE, "%s got up (state=%s).",
                    node.getName().c_str(), cstate.getLabel());
            setState(node, cstate);
          }
        } catch (const TimeoutException& e) {
          LogFile::debug("%s timeout for state", node.getName().c_str());
        }
      }
    } catch (const NSMNotConnectedException&) {
      if (cstate != Enum::UNKNOWN) {
        logging(getNode(), LogFile::ERROR, "%s got down.", node.getName().c_str());
        setState(node, Enum::UNKNOWN);
        synchronized = false;
        state_new = HVState::ERROR_ES;
      }
    }
    if (cstate.isStable()) {
      if (state_new == HVState::UNKNOWN) {
        state_new = cstate;
      }
      if (state_new != cstate) {
        synchronized = false;
      }
    } else if (cstate.isError()) {
      state_new = HVState::ERROR_ES;
    }
  }
  if (state_new == HVState::ERROR_ES ||
      (synchronized && state_new != HVState::UNKNOWN && state != state_new)) {
    setState(state_new);
    reply(NSMMessage(NSMCommand::OK, state_new.getLabel()));
  }
}

void HVMasterCallback::timeout(NSMCommunicator&) throw()
{
  monitor();
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
  distribute(NSMMessage(HVCommand::RECOVER));
}
