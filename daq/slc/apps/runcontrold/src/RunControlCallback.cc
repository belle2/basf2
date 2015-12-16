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
  distribute_r(NSMMessage(RCCommand::ABORT));
  for (size_t i = 0; i < m_lrc_v.size(); i++) {
    try {
      set(m_lrc_v[i], "used", 0);
    } catch (const IOException& e) {
      LogFile::debug(e.what());
    }
  }
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

void RunControlCallback::setConfig(RCNode& node, const std::string& config) throw()
{
  node.setConfig(config);
  DBObjectList& objs(getDBObject().getObjects("node"));
  for (DBObjectList::iterator it = objs.begin(); it != objs.end(); it++) {
    DBObject& o_node(*it);
    if (node.getName() == o_node.getText("name")) {
      if (o_node.hasObject("rcconfig")) {
        o_node("rcconfig").setPath(config);
      }
    }
  }
}

void RunControlCallback::ok(const char* nodename, const char* data) throw()
{
  LogFile::debug("OK from %s (state = %s)", nodename, data);
  RCState state(data);
  try {
    RCNode& node(findNode(nodename));
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

void RunControlCallback::error(const char* nodename, const char* data) throw()
{
  try {
    RCNode& node(findNode(nodename));
    logging(node, LogFile::ERROR, data);
    std::for_each(m_node_v.begin(), m_node_v.end(),
                  Recoveror(*this, NSMMessage(RCCommand::RECOVER)));
    setState(RCState::NOTREADY_S);
    m_restarting = false;
    //m_starttime = -1;
  } catch (const std::out_of_range& e) {
    LogFile::warning("ERROR from unknown node %s : %s", nodename, data);
  }
  monitor();
}

void RunControlCallback::fatal(const char* nodename, const char* data) throw()
{
  try {
    RCNode& node(findNode(nodename));
    logging(node, LogFile::FATAL, data);
    setState(RCState::ABORTING_RS);
    abort();
  } catch (const std::out_of_range& e) {
    LogFile::warning("ERROR from unknown node %s : %s", nodename, data);
  }
  monitor();
}

void RunControlCallback::log(const char* nodename, const DAQLogMessage& lmsg,
                             bool recorded) throw()
{
  try {
    RCNode& node(findNode(nodename));
    logging_imp(node, lmsg.getPriority(), lmsg.getDate(),
                lmsg.getMessage(), recorded);
  } catch (const std::out_of_range& e) {
    LogFile::warning("Log from unknown node %s : %s", nodename,
                     lmsg.getMessage().c_str());
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
  }
}

void RunControlCallback::boot(const DBObject& obj) throw(RCHandlerException)
{
  m_runno.setConfig(obj.getName());
  distribute(NSMMessage(RCCommand::BOOT));
}

void RunControlCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  obj.print();
  m_runno.setConfig(obj.getName());
  distribute(NSMMessage(RCCommand::LOAD));
  obj.print();
}

void RunControlCallback::start(int expno, int runno) throw(RCHandlerException)
{
  try {
    DBObject& obj(getDBObject());
    for (size_t i = 0; i < m_node_v.size(); i++) {
      RCNode& node(m_node_v[i]);
      std::string val;
      int count = 0;
      while (node.isUsed() && count < 10) {
        try {
          get(node, "rcconfig", val);
        } catch (const TimeoutException& e) {
          count++;
          continue;
        }
        break;
      }
      LogFile::info("%s config : %s", node.getName().c_str(), val.c_str());
      node.setConfig(val);
      std::string vname = StringUtil::form("node[%d]", (int)i);
      set(vname + ".rcconfig", val);
      (obj("node", i))("rcconfig").setPath(val);
    }
    if (expno == 0 || runno == 0) {
      if (getDB()) {
        DBInterface& db(*getDB());
        if (!db.isConnected()) db.connect();
        if (expno == 0) expno = m_runno.getExpNumber();
        m_runno = RunNumberTable(db).add(m_runno.getConfig(), expno, runno, 0);
        expno = m_runno.getExpNumber();
        runno = m_runno.getRunNumber();
        std::cout << "record_time : " << m_runno.getRecordTime() << std::endl;
        set("tstart", (int)m_runno.getRecordTime());
        set("ismaster", (int)true);
        std::string comment;
        get("comment", comment);
        obj.addText("comment", comment);
        std::string operators;
        get("operators", operators);
        obj.addText("opeeators", operators);
        dbrecord(obj, expno, runno);
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
  distribute(NSMMessage(RCCommand::RECOVER));
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
  if (m_restarttime > 0) {
    try {
      if (!m_restarting) {
        if (m_starttime > 0 && Time().get() - m_starttime > m_restarttime) {
          LogFile::info("Run automatically stopped due to exceeding run length");
          stop();
          m_restarting = true;
        }
      } else {
        bool all_ready = true;
        for (RCNodeIterator it = m_node_v.begin(); it != m_node_v.end(); it++) {
          RCNode& node(*it);
          if (!node.isUsed()) continue;
          RCState cstate(node.getState());
          if (cstate != RCState::READY_S) {
            all_ready = false;
            break;
          }
        }
        if (all_ready) {
          m_restarting = false;
          m_starttime = -1;
          LogFile::info("Run automatically starting");
          start(0, 0);
        }
      }
    } catch (const IOException& e) {
      LogFile::error("%s %s:%d", e.what(), __FILE__, __LINE__);
    }
  }
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
            std::string table = "";
            get(node, "dbtable", table, 1);
            set(StringUtil::tolower(node.getName()) + ".dbtable", table);
          }
        } catch (const TimeoutException& e) {
          LogFile::debug("%s timeout", node.getName().c_str());
        }
      }
    } catch (const NSMNotConnectedException&) {
      if (cstate != Enum::UNKNOWN) {
        logging(getNode(), LogFile::ERROR, "%s got down.", node.getName().c_str());
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
  DAQLogMessage log(node.getName(), pri, msg, date);
  LogFile::put(pri, msg);
  DBInterface& db(*getDB());
  try {
    if (log.getPriority() >= m_priority_db && !recorded) {
      if (!db.isConnected()) db.connect();
      DAQLogDB::createLog(db, m_logtable, log);
    }
  } catch (const DBHandlerException& e) {
    db.close();
    LogFile::error("DB errir : %s", e.what());
  }
  if ((getNode().getState() == RCState::RUNNING_S ||
       getNode().getState() == RCState::STARTING_TS) && pri >= LogFile::ERROR) {
    setState(RCState::NOTREADY_S);
  }
  if (log.getPriority() >= m_priority_global) {
    reply(NSMMessage(log, true));
  }
}

bool RunControlCallback::addAll(const DBObject& obj) throw()
{
  RCNodeList node_v;
  try {
    obj.print();
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
        LogFile::info("found rcconfig :%s %s", node.getName().c_str(), path.c_str());
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
    add(new NSMVHandlerRCConfig(*this, vname + ".rcconfig", node));
    //add(new NSMVHandlerText(vname + ".rcconfig", true, true, node.getConfig()));
    add(new NSMVHandlerRCState(*this, vname + ".rcstate", node));
    add(new NSMVHandlerRCRequest(*this, vname + ".rcrequest", node));
    add(new NSMVHandlerRCNodeUsed(*this, vname + ".used", node));
    vname = StringUtil::form("%s", StringUtil::tolower(node.getName()).c_str());
    add(new NSMVHandlerText(vname + ".dbtable", true, false, table));
    add(new NSMVHandlerRCConfig(*this, vname + ".rcconfig", node));
    //add(new NSMVHandlerText(vname + ".rcconfig", true, true, node.getConfig()));
    add(new NSMVHandlerRCState(*this, vname + ".rcstate", node));
    add(new NSMVHandlerRCRequest(*this, vname + ".rcrequest", node));
    add(new NSMVHandlerRCNodeUsed(*this, vname + ".used", node));
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
      if (cmd == RCCommand::CONFIGURE)
        m_msg.setData(node.getConfig());
      try {
        if (cmd == RCCommand::LOAD ||
            cmd == RCCommand::BOOT) {
          while (node.isSequential() &&
                 !m_callback.check(node.getName(), RCState::READY_S)) {
            try {
              m_callback.wait(node, RCCommand::OK, 1);
            } catch (const TimeoutException& e) {
              continue;
            } catch (const IOException& e) {
              LogFile::error("%s %s:%d", e.what(), __FILE__, __LINE__);
              //LogFile::error(e.what());
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
            m_callback.logging(m_callback.getNode(), LogFile::ERROR,
                               "%s is down.", node.getName().c_str());
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
        LogFile::error("%s %s:%d", e.what(), __FILE__, __LINE__);
        //LogFile::error(e.what());
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
    if (node.isUsed() && state != RCState::RECOVERING_RS
        && state != RCState::ABORTING_RS) {
      try {
        if (NSMCommunicator::send(m_msg)) {
          m_callback.setState(node, RCState::RECOVERING_RS);
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

void RunControlCallback::setExpNumber(int expno) throw()
{
  try {
    m_runno.setExpNumber(expno);
    int runno = RunNumberTable(*getDB()).getRunNumber(expno);
    std::string msg = StringUtil::form("Set exp no = %d (run no =%d)", expno, runno);
    LogFile::info(msg);
    set("runno", runno);
    reply(NSMMessage(DAQLogMessage(getNode().getName(), LogFile::INFO, msg), true));
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
      LogFile::info("Masked");
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
      LogFile::info("UnMasked");
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
  for (size_t i = 0; i < rc.size(); i++) {
    LogFile::debug("Local RC: " + rc[i]);
    m_lrc_v.push_back(RCNode(rc[i]));
  }
}
