#include "daq/slc/apps/hvmasterd/HVMasterCallback.h"
#include "daq/slc/apps/hvmasterd/HVMasterHandler.h"

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

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
  m_showall = false;
}

HVMasterCallback::~HVMasterCallback() throw()
{
}

void HVMasterCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  m_hvnode.setName(getNode().getName());
  m_hvnode.setState(HVState::OFF_S);
  configure(obj);
}

void HVMasterCallback::configure(const DBObject& obj) throw(RCHandlerException)
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
      /*
      if (o_node.hasObject("config")) {
        const std::string path = o_node("config").getPath();
        LogFile::info("found config :%s %s", node.getName().c_str(), path.c_str());
        node.setConfig(path);
      } else {
        LogFile::warning("Not found config");
      }
      */
      node_v.push_back(node);
    }
  } catch (const DBHandlerException& e) {
    throw (RCHandlerException("Failed to load db : %s", e.what()));
  }
  add(new NSMVHandlerText("hvstate", true, false, m_hvnode.getState().getLabel()));
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
    add(new NSMVHandlerHVNodeState(*this, vname + ".hvstate", node));
    add(new NSMVHandlerHVRequest(*this, vname + ".hvrequest", node));
    add(new NSMVHandlerHVNodeUsed(*this, vname + ".used", node));
    vname = StringUtil::form("%s", StringUtil::tolower(node.getName()).c_str());
    add(new NSMVHandlerText(vname + ".dbtable", true, false, table));
    add(new NSMVHandlerHVNodeState(*this, vname + ".hvstate", node));
    add(new NSMVHandlerHVRequest(*this, vname + ".hvrequest", node));
    add(new NSMVHandlerHVNodeUsed(*this, vname + ".used", node));
  }
}

void HVMasterCallback::monitor() throw(RCHandlerException)
{
  HVState state(getNode().getState());
  HVState state_new = HVState::UNKNOWN;
  bool failed = false;
  for (size_t i = 0; i < m_node_v.size(); i++) {
    HVNode& node(m_node_v[i]);
    if (!node.isUsed()) continue;
    HVState cstate(node.getState());
    HVState cstate_new;
    try {
      NSMCommunicator::connected(node.getName());
      try {
        std::string s;
        get(node, "hvstate", s, 1);
        cstate_new = HVState(s);
        if (cstate_new != cstate) {
          if (cstate == Enum::UNKNOWN) {
            log(LogFile::INFO, "%s got up (state=%s).",
                node.getName().c_str(), cstate_new.getLabel());
          }
          cstate = cstate_new;
          setState(node, cstate);
        }
      } catch (const TimeoutException& e) {
        LogFile::debug("%s timeout", node.getName().c_str());
      }
    } catch (const NSMNotConnectedException&) {
      if (cstate != Enum::UNKNOWN) {
        log(LogFile::ERROR, "%s got down.", node.getName().c_str());
        setState(node, Enum::UNKNOWN);
        setState(HVState::ERROR_ES);
        failed = true;
      }
    }
    if (state.isStable() && state != cstate) {
      if (cstate.isStable() && state_new.getId() > cstate.getId())
        state_new = cstate;
    } else if ((state.isTransition() || state.isError()) && state_new != cstate) {
      state_new = HVState::UNKNOWN;
    }
  }
  //if (failed) state_new = HVState::NOTREADY_S;
  if (state_new != HVState::UNKNOWN && state != state_new) {
    setState(state_new);
  }
  const std::string nodename = m_node_v[m_node_v.size() - 1].getName();
  if (getNode().getState() != HVState::OFF_S &&
      checkAll(nodename, HVState::OFF_S)) {
    setState(HVState::OFF_S);
  }
  if (getNode().getState() != HVState::STANDBY_S &&
      checkAll(nodename, HVState::STANDBY_S)) {
    setState(HVState::STANDBY_S);
  }
  if (getNode().getState() != HVState::PEAK_S &&
      checkAll(nodename, HVState::PEAK_S)) {
    setState(HVState::PEAK_S);
  }
}

void HVMasterCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  /*
  bool ready = false;
  while (!ready) {
    ready = true;
    for (HVNodeIterator it = m_node_v.begin(); it != m_node_v.end(); it++) {
      HVNode& node(*it);
      if (!node.isUsed()) continue;
      HVState cstate(node.getState());
      if (cstate != HVState::PEAK_S) {
        try {
          std::string s;
          get(node, "hvstate", s, 1);
          if ((cstate = HVState(s)) != Enum::UNKNOWN) {
            setState(node, cstate);
          }
        } catch (const TimeoutException& e) {
          LogFile::debug("%s timeout for state", node.getName().c_str());
        }
      }
      cstate = node.getState();
      if (cstate == HVState::OFF_S) {
        NSMCommunicator::send(NSMMessage(node, HVCommand::TURNON));
        log(LogFile::DEBUG, "TURNON >> " + node.getName());
        setState(node, HVState::TURNINGON_TS);
        ready = false;
      } else if (cstate == HVState::STANDBY_S) {
        NSMCommunicator::send(NSMMessage(node, HVCommand::PEAK));
        log(LogFile::DEBUG, "PEAK >> " + node.getName());
        setState(node, HVState::RAMPINGUP_TS);
        ready = false;
      } else if (!cstate.isStable()) {
        ready = false;
      }
    }
    if (ready) break;
    try {
      perform(wait(NSMNode(), NSMCommand::UNKNOWN, 1));
    } catch (const TimeoutException& e) {
      monitor();
    }
  }
  LogFile::debug("Load done");
  */
}

void HVMasterCallback::start(int expno, int runno) throw(RCHandlerException)
{
  LogFile::debug("run # = %04d.%04d.%03d", expno, runno, 0);
  LogFile::debug("Start done");
}

void HVMasterCallback::stop() throw(RCHandlerException)
{
  LogFile::debug("Stop done");
}

bool HVMasterCallback::resume(int subno) throw(RCHandlerException)
{
  return true;
}

bool HVMasterCallback::pause() throw(RCHandlerException)
{
  LogFile::debug("Pause done");
  return true;
}

void HVMasterCallback::recover(const DBObject& obj) throw(RCHandlerException)
{
  abort();
  load(obj);
  LogFile::debug("Recover done");
}

void HVMasterCallback::abort() throw(RCHandlerException)
{
  /*
  for (HVNodeIterator it = m_node_v.begin(); it != m_node_v.end(); it++) {
    HVNode& node(*it);
    if (!node.isUsed()) continue;
    //NSMCommunicator::send(NSMMessage(node, HVCommand::TURNOFF));
  }
  LogFile::debug("Abort done");
  */
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
    setState(node, HVState::ERROR_ES);
  } catch (const std::out_of_range& e) {
    LogFile::warning("ERROR from unknown node %s : %s", nodename, data);
  }
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

bool HVMasterCallback::perform(NSMCommunicator& com) throw()
{
  if (RCCallback::perform(com)) return true;
  try {
    NSMMessage msg(com.getMessage());
    HVCommand cmd(msg.getRequestName());
    if (cmd == HVCommand::UNKNOWN) return false;
    const HVState state(m_hvnode.getState());
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

void HVMasterCallback::distribute(NSMMessage msg) throw()
{
  std::for_each(m_node_v.begin(), m_node_v.end(), Distributor(*this, msg));
}

void HVMasterCallback::distribute_r(NSMMessage msg) throw()
{
  std::for_each(m_node_v.rbegin(), m_node_v.rend(), Distributor(*this, msg));
}

HVNode& HVMasterCallback::findNode(const std::string& name)
throw(std::out_of_range)
{
  for (HVNodeIterator it = m_node_v.begin(); it != m_node_v.end(); it++) {
    if (it->getName() == name) return *it;
  }
  throw (std::out_of_range(std::string("no node ") + name + " was found"));
}

bool HVMasterCallback::check(const std::string& node, const HVState& state)
throw()
{
  for (HVNodeIterator it = m_node_v.begin(); it != m_node_v.end(); it++) {
    if (it->getName() == node) return true;
    if (it->isUsed() && it->getState() != state) {
      return false;
    }
  }
  return true;
}

bool HVMasterCallback::checkAll(const std::string& node, const HVState& state) throw()
{
  for (size_t i = 0; i < m_node_v.size(); i++) {
    HVNode& cnode(m_node_v[i]);
    if (cnode.isUsed() && cnode.getState() != state) {
      return false;
    }
    if (cnode.getName() == node) return true;
  }
  return true;
}

void HVMasterCallback::setState(const HVState& state) throw()
{
  if (m_hvnode.getState() != state) {
    LogFile::debug("state transit : %s >> %s",
                   m_hvnode.getState().getLabel(), state.getLabel());
    m_hvnode.setState(state);
    try {
      set("hvstate", state.getLabel());
    } catch (const std::exception& e) {
      LogFile::error(e.what());
    }
    RCState rcstate(getNode().getState());
    if (rcstate.isStable()) {
      if (state != HVState::PEAK_S) {
        if (rcstate == RCState::RUNNING_S) {
          log(LogFile::FATAL, "HV got down during run");
          RCCallback::setState(RCState::ERROR_ES);
        } else {
          RCCallback::setState(RCState::NOTREADY_S);
        }
      } else if (state == HVState::PEAK_S) {
        RCCallback::setState(RCState::READY_S);
      }
    }
  }
  //reply(NSMMessage(NSMCommand::OK, state.getLabel()));
}

void HVMasterCallback::setState(HVNode& node, const HVState& state) throw()
{
  node.setState(state);
  std::string vname = StringUtil::tolower(node.getName()) + ".hvstate";
  set(vname, state.getLabel());
}

void HVMasterCallback::setConfig(HVNode& node, const std::string& config) throw()
{
  node.setConfig(config);
  std::string vname = StringUtil::tolower(node.getName()) + ".hvconfig";
  set(vname, config);
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
            m_callback.log(LogFile::ERROR, node.getName() + " is down.");
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
