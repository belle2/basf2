#include "daq/slc/apps/runcontrold/RunControlCallback.h"

#include <daq/slc/apps/runcontrold/RunControlHandler.h>

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

RunControlCallback::RunControlCallback()
{
  setAutoReply(false);
  m_showall = false;
  m_restarttime = -1;
  m_starttime = -1;
  m_restarting = false;
}

void RunControlCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  add(new NSMVHandlerRCUsed(*this, "used", true));
  DBInterface& db(*getDB());
  try {
    db.connect();
    RunNumberTable rntable(db);
    rntable.create();
    m_runno.setExpNumber(rntable.getExpNumber());
    m_runno.setRunNumber(rntable.getRunNumber(m_runno.getExpNumber()));
    m_runno.setStart(false);
  } catch (const DBHandlerException& e) {
  }
  db.close();
  if (!addAll(obj)) {
    throw (RCHandlerException("Failed to initialize (config=%s)", obj.getName().c_str()));
  }
  for (size_t i = 0; i < m_lrc_v.size(); i++) {
    try {
      set(m_lrc_v[i], "used", 0);
    } catch (const IOException& e) {
      LogFile::debug(e.what());
    }
  }
  getNode().setUsed(true);
}

void RunControlCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  if (addAll(obj)) {
    distribute(NSMMessage(RCCommand::CONFIGURE));
  } else {
    throw (RCHandlerException("Failed to configure (config=%s)", obj.getName().c_str()));
  }
}

void RunControlCallback::setState(NSMNode& node, const RCState& state) throw()
{
  node.setState(state);
  std::string vname = StringUtil::tolower(node.getName()) + ".rcstate";
  set(vname, state.getLabel());
  for (size_t i = 0; i < m_node_v.size(); i++) {
    RCNode& rcnode(m_node_v[i]);
    if (rcnode.getName() == node.getName()) {
      std::string vname = StringUtil::form("node[%d].rcstate", (int)i);
      set(vname, state.getLabel());
      return;
    }
  }
}

void RunControlCallback::ok(const char* nodename, const char* data) throw()
{
  log(LogFile::DEBUG, "OK from %s (state = %s)", nodename, data);
  RCState state(data);
  try {
    RCNode& node(findNode(nodename));
    if (node.isUsed()) {
      if (state == NSMState::UNKNOWN) {
        LogFile::warning("got unknown state (%s) from %s", data, nodename);
      } else {
        setState(node, state);
      }
    } else {
      LogFile::debug("node used");
    }
  } catch (const std::out_of_range& e) {
    LogFile::debug(e.what());
  }
  //monitor();
}

void RunControlCallback::error(const char* nodename, const char* data) throw()
{
  //log(LogFile::DEBUG, "ERROR from %s (state = %s)", nodename, data);
  try {
    RCNode& node(findNode(nodename));
    logging(node, LogFile::ERROR, data);
    //reply(NSMMessage(RCCommand::STOP, "Error due to error on " + node.getName()));
    //stop();
    setState(RCState::ERROR_ES);
    m_starttime = -1;
    m_restarting = false;
  } catch (const std::out_of_range& e) {
    LogFile::warning("ERROR from unknown node %s : %s", nodename, data);
  }
  //monitor();
}

void RunControlCallback::fatal(const char* nodename, const char* data) throw()
{
  try {
    RCNode& node(findNode(nodename));
    logging(node, LogFile::FATAL, data);
    setState(RCState::ERROR_ES);
    m_starttime = -1;
    m_restarting = false;
    //setState(RCState::ABORTING_RS);
    //reply(NSMMessage(RCCommand::ABORT, "Aborting due to error on " + node.getName()));
    //abort();
  } catch (const std::out_of_range& e) {
    LogFile::warning("ERROR from unknown node %s : %s", nodename, data);
  }
  //monitor();
}

void RunControlCallback::boot(const DBObject& obj) throw(RCHandlerException)
{
  m_runno.setConfig(obj.getName());
  distribute(NSMMessage(RCCommand::BOOT));
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
        if (expno == 0) expno = m_runno.getExpNumber();
        m_runno = RunNumberTable(db).add(m_runno.getConfig(), expno, runno, 0);
        expno = m_runno.getExpNumber();
        runno = m_runno.getRunNumber();
        setRunNumbers(expno, runno);
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
    log(LogFile::ERROR, e.what());
  }
  m_starttime = Time().get();
}

void RunControlCallback::stop() throw(RCHandlerException)
{
  postRun();
  distribute(NSMMessage(RCCommand::STOP));
  m_restarting = false;
  m_starttime = -1;
}

void RunControlCallback::recover(const DBObject&) throw(RCHandlerException)
{
  distribute_r(NSMMessage(RCCommand::RECOVER));
  m_restarting = false;
  m_starttime = -1;
}

bool RunControlCallback::pause() throw(RCHandlerException)
{
  distribute(NSMMessage(RCCommand::PAUSE));
  return true;
}

bool RunControlCallback::resume(int subno) throw(RCHandlerException)
{
  distribute(NSMMessage(RCCommand::RESUME, subno));
  return true;
}

void RunControlCallback::abort() throw(RCHandlerException)
{
  postRun();
  distribute_r(NSMMessage(RCCommand::ABORT));
  m_restarting = false;
  m_starttime = -1;
}

void RunControlCallback::monitor() throw(RCHandlerException)
{
  RCState state(getNode().getState());
  RCState state_new = state.next();
  bool failed = false;
  for (size_t i = 0; i < m_node_v.size(); i++) {
    RCNode& node(m_node_v[i]);
    if (!node.isUsed()) continue;
    RCState cstate(node.getState());
    RCState cstate_new;
    try {
      NSMCommunicator::connected(node.getName());
      try {
        if (cstate == Enum::UNKNOWN || cstate.isError() || state.isStable()) {
          std::string s;
          get(node, "rcstate", s, 5);
          cstate_new = RCState(s);
        } else {
          cstate_new = cstate;
        }
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
        if (state == RCState::RUNNING_S) {
          //stop();
        }
        setState(RCState::ERROR_ES);
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
  if (getNode().getState() != RCState::ERROR_ES) {
    if (failed) state_new = RCState::NOTREADY_S;
    if (state_new != RCState::UNKNOWN && state != state_new) {
      setState(state_new);
    }
    const std::string nodename = m_node_v[m_node_v.size() - 1].getName();
    if (getNode().getState() != RCState::NOTREADY_S &&
        checkAll(nodename, RCState::NOTREADY_S)) {
      setState(RCState::NOTREADY_S);
    }
    if (getNode().getState() != RCState::READY_S &&
        checkAll(nodename, RCState::READY_S)) {
      setState(RCState::READY_S);
    }
    if (getNode().getState() != RCState::PAUSED_S &&
        checkAll(nodename, RCState::PAUSED_S)) {
      setState(RCState::PAUSED_S);
    }
    if (getNode().getState() != RCState::RUNNING_S &&
        checkAll(nodename, RCState::RUNNING_S)) {
      setState(RCState::RUNNING_S);
    }
    if (getNode().getState() == RCState::RUNNING_S &&
        !checkAll(nodename, RCState::RUNNING_S)) {
      setState(RCState::ERROR_ES);
      stop();
    }
  }
}

void RunControlCallback::distribute(NSMMessage msg) throw()
{
  std::for_each(m_node_v.begin(), m_node_v.end(), Distributor(*this, msg));
}

void RunControlCallback::distribute_r(NSMMessage msg) throw()
{
  std::for_each(m_node_v.rbegin(), m_node_v.rend(), Distributor(*this, msg));
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
      db.close();
    }
  } catch (const std::exception& e) {
    log(LogFile::ERROR, e.what());
  }
}

RCNode& RunControlCallback::findNode(const std::string& name) throw(std::out_of_range)
{
  for (size_t i = 0; i < m_node_v.size(); i++) {
    RCNode& node(m_node_v[i]);
    if (node.getName() == name) return node;
  }
  throw (std::out_of_range(StringUtil::form("no node %s was found", name.c_str())));
}

bool RunControlCallback::check(const std::string& node, const RCState& state) throw()
{
  for (size_t i = 0; i < m_node_v.size(); i++) {
    RCNode& cnode(m_node_v[i]);
    if (cnode.getName() == node) return true;
    if (cnode.isUsed() && cnode.getState() != state) {
      return false;
    }
  }
  return true;
}

bool RunControlCallback::checkAll(const std::string& node, const RCState& state) throw()
{
  for (size_t i = 0; i < m_node_v.size(); i++) {
    RCNode& cnode(m_node_v[i]);
    if (cnode.isUsed() && cnode.getState() != state) {
      return false;
    }
    if (cnode.getName() == node) return true;
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
  logging_imp(node, pri, Date(), buf);
}

void RunControlCallback::logging_imp(const NSMNode& node, LogFile::Priority pri,
                                     const Date& date, const std::string& msg)
{
  DAQLogMessage log(node.getName(), pri, msg, date);
  LogFile::put(pri, msg);
  if ((getNode().getState() == RCState::RUNNING_S ||
       getNode().getState() == RCState::STARTING_TS) && pri >= LogFile::ERROR) {
    setState(RCState::NOTREADY_S);
  }
  if (log.getPriority() >= m_priority_global) {
    //  reply(NSMMessage(log));
  }
  RCCallback::log(pri, msg);
}

bool RunControlCallback::addAll(const DBObject& obj) throw()
{
  RCNodeList node_v;
  const DBObjectList& objs(obj.getObjects("node"));
  try {
    obj.print();
    for (size_t i = 0; i < objs.size(); i++) {
      const DBObject& o_node(objs[i]);
      RCNode node(o_node.getText("name"));
      try {
        RCNode& node_i(findNode(node.getName()));
        node = node_i;
      } catch (const std::out_of_range& e) {
        log(LogFile::INFO, "New node : " + node.getName());
      }
      node.setUsed(o_node.getBool("used"));
      node.setSequential(o_node.getBool("sequential"));
      if (o_node.hasText("rcconfig")) {
        const std::string path = o_node.getText("rcconfig");
        log(LogFile::DEBUG, "found rcconfig :%s %s", node.getName().c_str(), path.c_str());
        node.setConfig(path);
      } else {
        log(LogFile::WARNING, "Not found rcconfig");
      }
      node_v.push_back(node);
    }
  } catch (const DBHandlerException& e) {
    log(LogFile::ERROR, "Failed to load db : %s", e.what());
    return false;
  }
  m_node_v = node_v;
  add(new NSMVHandlerInt("nnodes", true, false, (int)m_node_v.size()));
  add(new NSMVHandlerRCExpNumber(*this, "expno", m_runno.getExpNumber()));
  add(new NSMVHandlerInt("runno", true, false, m_runno.getRunNumber()));
  add(new NSMVHandlerInt("subno", true, false, 0));
  add(new NSMVHandlerInt("tstart", true, false, (int)m_runno.getRecordTime()));
  add(new NSMVHandlerInt("ismaster", true, false, (int)true));
  add(new NSMVHandlerText("operators", true, true, m_operators));
  add(new NSMVHandlerText("comment", true, true, m_comment));
  for (size_t i = 0; i < m_node_v.size(); i++) {
    RCNode& node(m_node_v[i]);
    std::string vname = StringUtil::form("node[%d]", (int)i);
    add(new NSMVHandlerText(vname + ".name", true, false, node.getName()));
    std::string table = "";
    try {
      get(node, "dbtable", table, 1);
    } catch (const TimeoutException& e) {}
    add(new NSMVHandlerText(vname + ".rcconfig", true, false, node.getConfig()));
    add(new NSMVHandlerRCState(*this, vname + ".rcstate", node));
    add(new NSMVHandlerRCRequest(*this, vname + ".rcrequest", node));
    add(new NSMVHandlerRCNodeUsed(*this, vname + ".used", node));
    const DBObject& o_node(objs[i]);
    if (!o_node.getBool("used")) {
      set(vname + ".rcstate", "OFF");
      set(vname + ".used", false);
    }
    vname = StringUtil::form("%s", StringUtil::tolower(node.getName()).c_str());
    add(new NSMVHandlerText(vname + ".dbtable", true, false, table));
    add(new NSMVHandlerText(vname + ".rcconfig", true, false, node.getConfig()));
    add(new NSMVHandlerRCState(*this, vname + ".rcstate", node));
    add(new NSMVHandlerRCRequest(*this, vname + ".rcrequest", node));
    add(new NSMVHandlerRCNodeUsed(*this, vname + ".used", node));
    if (!o_node.getBool("used")) {
      set(vname + ".rcstate", "OFF");
      set(vname + ".used", false);
    }
  }
  add(new NSMVHandlerText("log.dbtable",  true, false, m_logtable));
  return true;
}

void RunControlCallback::Distributor::operator()(RCNode& node) throw()
{
  if (!m_enabled) return;
  m_msg.setNodeName(node);
  RCCommand cmd(m_msg.getRequestName());
  if (m_msg.getNodeName() == node.getName()) {
    if (node.isUsed()) {
      try {
        if (cmd == RCCommand::CONFIGURE) {
          m_msg.setData(node.getConfig());
        } else if (cmd == RCCommand::LOAD) {
          while (node.isSequential() &&
                 !m_callback.check(node.getName(), RCState::READY_S)) {
            try {
              //NSMCommunicator& com(m_callback.wait(node, RCCommand::OK, 1));
              NSMCommunicator& com(m_callback.wait(NSMNode(), RCCommand::UNKNOWN, 1));
              NSMMessage msg = com.getMessage();
              RCCommand cmd2(msg.getRequestName());
              if (cmd2 == NSMCommand::OK) {
                m_callback.ok(msg.getNodeName(), msg.getData());
                continue;
              } else if (cmd2 == NSMCommand::VSET) {
                m_callback.perform(com);
                continue;
              } else if (cmd2 == NSMCommand::VGET) {
                m_callback.perform(com);
                continue;
              } else if (cmd2 == RCCommand::ABORT) {
                LogFile::debug("ABORTING");
                m_callback.setState(RCState::ABORTING_RS);
                m_callback.abort();
                m_enabled = false;
                return;
              } else {
                continue;
              }
            } catch (const TimeoutException& e) {
              continue;
            } catch (const IOException& e) {
              m_callback.log(LogFile::ERROR, "IOError %s", e.what());
              m_enabled = false;
              return;
            }
          }
        }
        if (NSMCommunicator::send(m_msg)) {
          RCState tstate = RCCommand(m_msg.getRequestName()).nextTState();
          if (tstate != Enum::UNKNOWN)
            m_callback.setState(node, tstate);
        } else {
          if (node.getState() != RCState::UNKNOWN) {
            m_callback.log(LogFile::ERROR, "%s is down.", node.getName().c_str());
          }
          m_callback.setState(node, RCState::UNKNOWN);
          m_callback.setState(RCState::NOTREADY_S);
          if (cmd == RCCommand::LOAD && node.isSequential())
            m_enabled = false;
        }
      } catch (const NSMHandlerException& e) {
        LogFile::fatal("Failed to NSM2 request");
        m_enabled = false;
      } catch (const IOException& e) {
        LogFile::error(e.what());
        m_enabled = false;
        m_callback.setState(RCState::NOTREADY_S);
      }
    }
  }
}

void RunControlCallback::Recoveror::operator()(RCNode& node) throw()
{
  if (!m_enabled) return;
  m_msg.setNodeName(node);
  RCCommand cmd = RCCommand::RECOVER;
  if (m_msg.getNodeName() == node.getName()) {
    RCState state(node.getState());
    if (node.isUsed() && state == RCState::NOTREADY_S) {
      try {
        if (NSMCommunicator::send(m_msg)) {
          m_callback.setState(node, RCState::RECOVERING_RS);
        }
      } catch (const NSMHandlerException& e) {
        LogFile::fatal("Failed to NSM2 request");
        m_enabled = false;
      } catch (const IOException& e) {
        m_callback.log(LogFile::ERROR, "IOError %s", e.what());
        m_enabled = false;
        m_callback.setState(RCState::NOTREADY_S);
      }
    }
  }
}

void RunControlCallback::setExpNumber(int expno) throw()
{
  try {
    m_runno.setExpNumber(expno);
    int runno = RunNumberTable(*getDB()).getRunNumber(expno);
    std::string msg = StringUtil::form("Set exp no = %d (run no =%d)", expno, runno);
    set("runno", runno);
    log(LogFile::INFO, msg);
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
  }
}

bool RunControlCallback::getRCUsed() throw()
{
  return getNode().isUsed();
}

bool RunControlCallback::setRCUsed(int val) throw()
{
  int used = (int)getNode().isUsed();
  if (val != used) {
    getNode().setUsed(val > 0);
    if (val == 0) {
      abort();
      RCCallback::setState(RCState::OFF_S);
      for (size_t i = 0; i < m_lrc_v.size(); i++) {
        try {
          set(m_lrc_v[i], "used", 1);
        } catch (const IOException& e) {
          LogFile::debug(e.what());
        }
      }
    } else {
      RCCallback::setState(RCState::NOTREADY_S);
      for (size_t i = 0; i < m_lrc_v.size(); i++) {
        try {
          set(m_lrc_v[i], "used", 0);
        } catch (const IOException& e) {
          LogFile::debug(e.what());
        }
      }
    }
  }
  return true;
}

void RunControlCallback::setLocalRunControls(const StringList& rc)
{
  m_lrc_v.clear();
  for (size_t i = 0; i < rc.size(); i++) {
    m_lrc_v.push_back(RCNode(rc[i]));
  }
}
