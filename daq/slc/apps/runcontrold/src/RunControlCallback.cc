#include "daq/slc/apps/runcontrold/RunControlCallback.h"

#include <daq/slc/apps/runcontrold/RunControlHandler.h>

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/Date.h>

#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <algorithm>

using namespace Belle2;

RunControlCallback::RunControlCallback(int port)
{
  m_port = port;
  setAutoReply(false);
  m_showall = false;
}

void RunControlCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  DBInterface& db(*getDB());
  try {
    db.connect();
    m_runno.setExpNumber(RunNumberTable(db).getExpNumber());
    m_runno.setRunNumber(RunNumberTable(db).getRunNumber(m_runno.getExpNumber()));
    m_runno.setStart(false);
  } catch (const DBHandlerException& e) {
  }
  db.close();
  if (!addAll(obj)) {
    throw (RCHandlerException("Failed to initialize (config=%s)", obj.getName().c_str()));
  }
}

void RunControlCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  if (addAll(obj)) {
    distribute(NSMMessage(RCCommand::CONFIGURE, m_port));
  } else {
    throw (RCHandlerException("Failed to configure (config=%s)", obj.getName().c_str()));
  }
}

void RunControlCallback::setState(NSMNode& node, const RCState& state)
throw()
{
  node.setState(state);
  std::string vname = StringUtil::tolower(node.getName()) + ".rcstate";
  set(vname, state.getLabel());
}

void RunControlCallback::ok(const char* nodename, const char* data) throw()
{
  LogFile::debug("OK from %s (state = %s)", nodename, data);
  RCState state(data);
  try {
    RCNode& node(findNode(nodename));
    if (state == NSMState::UNKNOWN) {
      LogFile::warning("got unknown state (%s) from %s", data, nodename);
    } else {
      setState(node, state);
    }
  } catch (const std::out_of_range& e) {}
  update();
}

void RunControlCallback::error(const char* nodename, const char* data) throw()
{
  try {
    RCNode& node(findNode(nodename));
    logging(node, LogFile::ERROR, data);
    setState(node, RCState::NOTREADY_S);
    setState(RCState::NOTREADY_S);
  } catch (const std::out_of_range& e) {
    LogFile::warning("ERROR from unknown node %s : %s", nodename, data);
  }
  update();
}

void RunControlCallback::log(const char* nodename, const DAQLogMessage& lmsg,
                             bool recorded) throw()
{
  try {
    RCNode& node(findNode(nodename));
    logging_imp(node, lmsg.getPriority(), lmsg.getDate(),
                "log form " + node.getName() + " : " +
                lmsg.getMessage(), recorded);
    reply(NSMMessage(lmsg));
  } catch (const std::out_of_range& e) {
    LogFile::warning("Log from unknown node %s : %s", nodename,
                     lmsg.getMessage().c_str());
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
  }
}

void RunControlCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  m_runno.setConfig(obj.getName());
  distribute(NSMMessage(RCCommand::LOAD));
}

void RunControlCallback::start(int expno, int runno) throw(RCHandlerException)
{
  try {
    if (expno == 0 || runno == 0) {
      if (getDB()) {
        DBInterface& db(*getDB());
        if (!db.isConnected()) db.connect();
        m_runno = RunNumberTable(db).add(m_runno.getConfig(), expno, runno, 0);
        expno = m_runno.getExpNumber();
        runno = m_runno.getRunNumber();
        set("tstart", (int)m_runno.getRecordTime());
        set("ismaster", (int)true);
      } else {
        throw (RCHandlerException("DB is not available"));
      }
    } else {
      set("ismaster", (int)false);
    }
    set("expno", expno);
    set("runno", runno);
    int pars[2] = {expno, runno};
    distribute(NSMMessage(RCCommand::START, 2, pars));
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
  }
}

void RunControlCallback::stop() throw(RCHandlerException)
{
  postRun();
  distribute(NSMMessage(RCCommand::STOP));
}

void RunControlCallback::recover(const DBObject&) throw(RCHandlerException)
{
  distribute(NSMMessage(RCCommand::RECOVER));
}

void RunControlCallback::pause() throw(RCHandlerException)
{
  distribute(NSMMessage(RCCommand::PAUSE));
}

void RunControlCallback::resume(int subno) throw(RCHandlerException)
{
  distribute(NSMMessage(RCCommand::RESUME, subno));
}

void RunControlCallback::abort() throw(RCHandlerException)
{
  postRun();
  distribute_r(NSMMessage(RCCommand::ABORT));
}

void RunControlCallback::timeout(NSMCommunicator&) throw()
{
  update();
}

void RunControlCallback::update() throw()
{
  RCState state(getNode().getState());
  RCState state_new = state.next();
  bool failed = false;
  for (RCNodeIterator it = m_node_v.begin(); it != m_node_v.end(); it++) {
    RCNode& node(*it);
    if (!node.isUsed()) continue;
    RCState cstate(node.getState());
    try {
      NSMCommunicator::connected(node.getName());
      if (cstate == Enum::UNKNOWN) {
        try {
          std::string s;
          get(node, "rcstate", s, 1);
          if ((cstate = RCState(s)) != Enum::UNKNOWN) {
            logging(getNode(), LogFile::NOTICE, "%s got up (state=%s).",
                    node.getName().c_str(), cstate.getLabel());
            setState(node, cstate);
          }
        } catch (const TimeoutException& e) {
          LogFile::debug("%s timeout", node.getName().c_str());
        }
      }
    } catch (const NSMNotConnectedException&) {
      if (cstate != Enum::UNKNOWN) {
        logging(getNode(), LogFile::WARNING, "%s got down.", node.getName().c_str());
        setState(node, Enum::UNKNOWN);
        failed = true;
      }
    }
    if (state.isStable() && state != cstate) {
      if (cstate.isStable() && state_new.getId() > cstate.getId())
        state_new = cstate;
    } else if ((state.isTransition() || state.isError() ||
                state.isRecovering()) && state_new != cstate) {
      state_new = RCState::UNKNOWN;
    }
  }
  if (failed) state_new = RCState::NOTREADY_S;
  if (state_new != RCState::UNKNOWN && state != state_new) {
    setState(state_new);
    reply(NSMMessage(NSMCommand::OK, state_new.getLabel()));
  }
}

void RunControlCallback::distribute(NSMMessage msg) throw()
{
  std::for_each(m_node_v.begin(), m_node_v.end(), Distributer(*this, msg));
}

void RunControlCallback::distribute_r(NSMMessage msg) throw()
{
  std::for_each(m_node_v.rbegin(), m_node_v.rend(), Distributer(*this, msg));
}

void RunControlCallback::postRun() throw()
{
  try {
    int ismaster = 0;
    get("ismaster", ismaster);
    if (ismaster && m_runno.isStart() && getDB()) {
      DBInterface& db(*getDB());
      db.connect();
      m_runno.setStart(false);
      RunNumberTable(db).add(m_runno);
    }
  } catch (const std::exception& e) {
    LogFile::error(e.what());
  }
}

RCNode& RunControlCallback::findNode(const std::string& name) throw(std::out_of_range)
{
  for (RCNodeIterator it = m_node_v.begin(); it != m_node_v.end(); it++) {
    if (it->getName() == name) return *it;
  }
  throw (std::out_of_range(StringUtil::form("no node %s was found", name.c_str())));
}

bool RunControlCallback::check(const std::string& node, const RCState& state) throw()
{
  for (RCNodeIterator it = m_node_v.begin(); it != m_node_v.end(); it++) {
    if (it->getName() == node) return true;
    if (it->isUsed() && it->getState() != state) {
      return false;
    }
  }
  return true;
}

void RunControlCallback::logging(const NSMNode& node, LogFile::Priority pri, const char* text, ...)
{
  static char buf[1024 * 100];
  va_list ap;
  va_start(ap, text);
  vsprintf(buf, text, ap);
  va_end(ap);
  logging_imp(node, pri, Date(), buf, false);
}

void RunControlCallback::logging_imp(const NSMNode& node, LogFile::Priority pri,
                                     const Date& date, const std::string& msg,
                                     bool recorded)
{
  const DAQLogMessage log(node.getName(), pri, msg, date);
  LogFile::put(pri, msg);
  DBInterface& db(*getDB());
  try {
    if (log.getPriority() >= m_priority_db && !recorded) {
      if (!db.isConnected()) db.connect();
      //LoggerObjectTable(getDB()).add(log, true);
    }
  } catch (const DBHandlerException& e) {
    db.close();
    LogFile::error("DB errir : %s", e.what());
  }
  if (log.getPriority() >= m_priority_global) {
    reply(NSMMessage(log, true));
  }
}

bool RunControlCallback::addAll(const DBObject& obj) throw()
{
  RCNodeList node_v;
  try {
    const DBObjectList& objs(obj.getObjects("node"));
    for (DBObjectList::const_iterator it = objs.begin(); it != objs.end(); it++) {
      const DBObject& o_node(*it);
      RCNode node(o_node.getText("name"));
      try {
        RCNode& node_i(findNode(node.getName()));
        node = node_i;
      } catch (const std::out_of_range& e) {
        LogFile::info("New node : " + node.getName());
      }
      node.setUsed(o_node.getBool("used"));
      node.setSequential(o_node.getBool("sequential"));
      if (o_node.hasObject("rcconfig")) {
        const std::string path = o_node("rcconfig").getPath();
        LogFile::info("found rcconfig :%s", path.c_str());
        node.setConfig(path);
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
  add(new NSMVHandlerInt("expno", true, true, m_runno.getExpNumber()));
  add(new NSMVHandlerInt("runno", true, false, m_runno.getRunNumber()));
  add(new NSMVHandlerInt("subno", true, false, 0));
  add(new NSMVHandlerInt("tstart", true, false, (int)m_runno.getRecordTime()));
  add(new NSMVHandlerInt("ismaster", true, false, (int)true));
  add(new NSMVHandlerText("operators", true, true, m_operators));
  add(new NSMVHandlerText("comment",  true, true, m_comment));
  for (size_t i = 0; i < m_node_v.size(); i++) {
    RCNode& node(m_node_v[i]);
    std::string vname = StringUtil::form("node[%d]", (int)i);
    add(new NSMVHandlerText(vname + ".name", true, false, node.getName()));
    add(new NSMVHandlerRCConfig(*this, vname + ".rcconfig", node));
    add(new NSMVHandlerRCState(*this, vname + ".rcstate", node));
    add(new NSMVHandlerRCRequest(*this, vname + ".rcrequest", node));
    add(new NSMVHandlerRCUsed(*this, vname + ".used", node));
    vname = StringUtil::form("%s", StringUtil::tolower(node.getName()).c_str());
    add(new NSMVHandlerRCConfig(*this, vname + ".rcconfig", node));
    add(new NSMVHandlerRCState(*this, vname + ".rcstate", node));
    add(new NSMVHandlerRCRequest(*this, vname + ".rcrequest", node));
    add(new NSMVHandlerRCUsed(*this, vname + ".used", node));
  }
  return true;
}

void RunControlCallback::Distributer::operator()(RCNode& node) throw()
{
  if (!m_enabled) return;
  m_msg.setNodeName(node);
  RCCommand cmd(m_msg.getRequestName());
  if (node.isUsed() && m_msg.getNodeName() == node.getName()) {
    if (cmd == RCCommand::CONFIGURE)
      m_msg.setData(node.getConfig());
    try {
      while (cmd == RCCommand::LOAD && node.isSequential() &&
             !m_callback.check(node.getName(), RCState::READY_S)) {
        try {
          m_callback.wait(node, RCCommand::OK, 1);
        } catch (const TimeoutException& e) {
          continue;
        } catch (const IOException& e) {
          LogFile::error(e.what());
          m_enabled = false;
          return;
        }
      }
      NSMCommunicator::send(m_msg);
      RCState tstate = RCCommand(m_msg.getRequestName()).nextTState();
      if (tstate != Enum::UNKNOWN)
        m_callback.setState(node, tstate);
    } catch (const NSMHandlerException& e) {
      LogFile::fatal("Failed to NSM2 request");
      m_enabled = false;
    } catch (const IOException& e) {
      LogFile::error(e.what());
      m_enabled = false;
      m_callback.setState(RCState::NOTREADY_S);
    }
  } else {
    LogFile::warning("Node '%s' is OFFLINE", node.getName().c_str());
    m_callback.setState(node, RCState::UNKNOWN);
    m_callback.setState(RCState::NOTREADY_S);
    if (cmd == RCCommand::LOAD && node.isSequential())
      m_enabled = false;
  }
}

