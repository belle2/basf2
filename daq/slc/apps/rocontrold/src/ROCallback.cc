#include "daq/slc/apps/rocontrold/ROCallback.h"

#include "daq/slc/apps/rocontrold/ropc_status.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/readout/ronode_info.h"
#include "daq/slc/readout/ronode_status.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/base/StringUtil.h>

#include <cstring>
#include <sstream>
#include <iostream>
#include <map>

#include <unistd.h>

namespace Belle2 {

  class NSMVHandlerROInputPort : public NSMVHandlerInt {
  public:
    NSMVHandlerROInputPort(ROController& con, const std::string& name)
      : NSMVHandlerInt(name, true, false), m_con(con) {}
    virtual ~NSMVHandlerROInputPort() throw() {}
    bool handleGetInt(int& val)
    {
      val = m_con.getControl().getInfo().getInputPort();
      return true;
    }
  private:
    ROController& m_con;
  };

  class NSMVHandlerROOutputPort : public NSMVHandlerInt {
  public:
    NSMVHandlerROOutputPort(ROController& con, const std::string& name)
      : NSMVHandlerInt(name, true, false), m_con(con) {}
    virtual ~NSMVHandlerROOutputPort() throw() {}
    bool handleGetInt(int& val)
    {
      val = m_con.getControl().getInfo().getOutputPort();
      return true;
    }
  private:
    ROController& m_con;
  };

  class NSMVHandlerCOPPERState : public NSMVHandlerText {
  public:
    NSMVHandlerCOPPERState(NSMNode& node, ROCallback& callback,
                           const std::string& name)
      : NSMVHandlerText(node.getName(), name, true, true, "UNKNOWN"),
        m_callback(callback), m_node(node) {}
    virtual ~NSMVHandlerCOPPERState() throw() {}
    virtual bool handleSetText(const std::string& val)
    {
      RCState state(val);
      if (state != RCState::UNKNOWN) {
        m_node.setState(state);
        if (state == RCState::ERROR_ES) {
          m_callback.setState(RCState::RECOVERING_RS);
          m_callback.recover(m_callback.getDBObject());
        }
      }
      return true;
    }
  private:
    ROCallback& m_callback;
    NSMNode& m_node;

  };

}

using namespace Belle2;

ROCallback::ROCallback(const NSMNode& runcontrol)
  : RCCallback(), m_runcontrol(runcontrol)
{
  system("killall basf2 eb0");
}

void ROCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  allocData(getNode().getName(), "ropc", ropc_revision);
  add(new NSMVHandlerFloat("loadavg", true, false, 0));
  configure(obj);
  setState(RCState::NOTREADY_S);
  const std::string path_shm = "/cpr_pause_resume";
  if (!m_memory.open(path_shm, sizeof(int))) {
    perror("shm_open");
    LogFile::error("Failed to open %s", path_shm.c_str());
  }
  char* buf = (char*)m_memory.map(0, sizeof(int));
  memset(buf, 0, sizeof(int));
}

void ROCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  try {
    const DBObjectList& stream0(obj.getObjects("stream0"));
    m_eb0.init(this, 0, "eb0", obj);
    m_stream0 = std::vector<Stream0Controller>();
    for (size_t i = 0; i < stream0.size(); i++) {
      m_stream0.push_back(Stream0Controller());
      std::string nodename = StringUtil::toupper(stream0[i].getText("name"));
      m_node.insert(std::make_pair(nodename, NSMNode(nodename)));
    }
    for (size_t i = 0; i < m_stream0.size(); i++) {
      std::string vname = stream0[i].hasText("name") ? stream0[i].getText("name") : stream0[i].getText("host");
      m_stream0[i].init(this, i + 2, vname, obj);
      vname = StringUtil::toupper(vname);
      //add(new NSMVHandlerCOPPERState(m_node[vname], *this, "rcstate"));
      vname = (m_stream0.size() == 1) ? "stream0" : StringUtil::form("stream0[%d]", (int)i);
      add(new NSMVHandlerROInputPort(m_stream0[i], vname + ".input.port"));
      add(new NSMVHandlerROOutputPort(m_stream0[i], vname + ".output.port"));
    }
  } catch (const std::out_of_range& e) {
    throw (RCHandlerException(e.what()));
  }
}

void ROCallback::term() throw()
{
  m_eb0.term();
  for (size_t i = 0; i < m_stream0.size(); i++) {
    m_stream0[0].term();
  }
}

void ROCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  if (!m_eb0.load(obj, 0)) {
    throw (RCHandlerException("Failed to boot eb0"));
  }
  log(LogFile::INFO, "Booted eb0");
  try_wait();
  for (size_t i = 0; i < m_stream0.size(); i++) {
    if (!m_stream0[i].load(obj, 10)) {
      throw (RCHandlerException("Faield to boot stream0-%d", (int)i));
    }
    log(LogFile::INFO, "Booted %d-th stream0", i);
    try_wait();
  }
  LogFile::debug("Booted stream1");
}

void ROCallback::start(int /*expno*/, int /*runno*/) throw(RCHandlerException)
{
  /*
  for (size_t i = 0; i < m_stream0.size(); i++) {
    if (!m_stream0[i].start(expno, runno)) {
      throw (RCHandlerException("Faield to start stream0-%d", (int)i));
      return;
    }
  }
  if (!m_stream1.start(expno, runno)) {
    throw (RCHandlerException("Faield to start stream1"));
  }
  */
}

bool ROCallback::pause() throw(RCHandlerException)
{
  LogFile::debug("Pausing");
  try {
    if (m_eb0.isUsed()) m_eb0.pause();
  } catch (const RCHandlerException& e) {
    log(LogFile::WARNING, "eb0 did not start : %s", e.what());
    return false;
  }
  for (size_t i = 0; i < m_stream0.size(); i++) {
    m_stream0[i].pause();
  }
  m_stream1.pause();
  return true;
}

bool ROCallback::resume(int subno) throw(RCHandlerException)
{
  LogFile::debug("Resuming");
  try {
    if (m_eb0.isUsed()) m_eb0.resume(subno);
  } catch (const RCHandlerException& e) {
    log(LogFile::WARNING, "eb0 did not restart : %s", e.what());
    return false;
  }
  for (size_t i = 0; i < m_stream0.size(); i++) {
    m_stream0[i].resume(subno);
  }
  m_stream1.resume(subno);
  return true;
}

void ROCallback::stop() throw(RCHandlerException)
{
  for (size_t i = 0; i < m_stream0.size(); i++) {
    m_stream0[i].stop();
  }
  m_eb0.stop();
}

void ROCallback::recover(const DBObject& obj) throw(RCHandlerException)
{
  for (std::map<std::string, NSMNode>::iterator it = m_node.begin();
       it != m_node.end(); it++) {
    NSMNode& node(it->second);
    LogFile::info(node.getName());
    NSMCommunicator::send(NSMMessage(node, RCCommand::ABORT));
  }
  abort();
  while (true) {
    bool notready_all = true;
    for (std::map<std::string, NSMNode>::iterator it = m_node.begin();
         it != m_node.end(); it++) {
      NSMNode& node(it->second);
      if (node.getState() != RCState::NOTREADY_S) {
        notready_all = false;
        break;
      }
    }
    if (notready_all) break;
    try {
      NSMCommunicator& com(wait(NSMNode(), NSMCommand::UNKNOWN, 10));
      NSMMessage msg(com.getMessage());
      RCCommand cmd(msg.getRequestName());
      if (cmd == NSMCommand::OK) {
        std::string nodename = msg.getNodeName();
        if (m_node.find(nodename) != m_node.end() && msg.getLength() > 0) {
          RCState s(msg.getData());
          if (s != NSMState::UNKNOWN)
            m_node[nodename].setState(s);
        }
      } else if (cmd == RCCommand::ABORT) {
        abort();
        setState(RCState::NOTREADY_S);
        return;
      } else if (cmd == RCCommand::STOP) {
      } else {
        perform(com);
      }
    } catch (const TimeoutException& e) {
      LogFile::debug("Timeout for wait OK from COPPERs");
    }
  }

  for (std::map<std::string, NSMNode>::iterator it = m_node.begin();
       it != m_node.end(); it++) {
    NSMNode& node(it->second);
    NSMCommunicator::send(NSMMessage(node, RCCommand::LOAD));
  }
  load(obj);

  while (true) {
    bool ready_all = true;
    for (std::map<std::string, NSMNode>::iterator it = m_node.begin();
         it != m_node.end(); it++) {
      NSMNode& node(it->second);
      if (node.getState() != RCState::READY_S) {
        ready_all = false;
        break;
      }
    }
    if (ready_all) {
      setState(RCState::READY_S);
      return;
    }
    try {
      NSMCommunicator& com(wait(NSMNode(), NSMCommand::UNKNOWN, 10));
      NSMMessage msg(com.getMessage());
      RCCommand cmd(msg.getRequestName());
      if (cmd == NSMCommand::OK) {
        std::string nodename = msg.getNodeName();
        if (m_node.find(nodename) != m_node.end() && msg.getLength() > 0) {
          RCState s(msg.getData());
          if (s != NSMState::UNKNOWN)
            m_node[nodename].setState(s);
        } else if (cmd == RCCommand::ABORT) {
          abort();
          setState(RCState::NOTREADY_S);
          return;
        } else if (cmd == RCCommand::STOP) {
        } else {
          perform(com);
        }
      }
    } catch (const TimeoutException& e) {
      LogFile::debug("Timeout for wait OK from COPPERs");
    }
  }
}

void ROCallback::abort() throw(RCHandlerException)
{
  m_stream1.abort();
  for (size_t i = 0; i < m_stream0.size(); i++) {
    m_stream0[i].abort();
  }
  m_eb0.abort();
}

void ROCallback::monitor() throw(RCHandlerException)
{
  NSMData& data(getData());
  if (data.isAvailable()) {
    ropc_status* nsm = (ropc_status*)data.get();
    if (getNode().getState() == RCState::RUNNING_S || getNode().getState() == RCState::READY_S) {
      for (size_t i = 0; i < m_stream0.size(); i++) {
        m_stream0[i].check();
        ronode_status& status(m_stream0[i].getFlow().monitor());
        memcpy(&(nsm->stream0[i]), &(status), sizeof(ronode_status));
      }
    } else {
      memset(nsm, 0, sizeof(ropc_status));
    }
    double loads[3];
    if (getloadavg(loads, 3) > 0) {
      nsm->loadavg = (float)loads[0];
      set("loadavg", (float)loads[0]);
    } else {
      nsm->loadavg = 0;
      set("loadavg", 0);
    }
    data.flush();
  }
  const RCState state(getNode().getState());
  for (size_t i = 0; i < m_stream0.size(); i++) {
    m_stream0[i].check();
  }
  m_eb0.check();
  if (state == RCState::RUNNING_S || state == RCState::STARTING_TS) {
    if (m_eb0.isUsed() && !m_eb0.getControl().isAlive()) {
      log(LogFile::ERROR, "eb0 was crashed");
      setState(RCState::ERROR_ES);
      return;
    }
    for (size_t i = 0; i < m_stream0.size(); i++) {
      if (m_stream0[i].isUsed() && !m_stream0[i].getControl().isAlive()) {
        log(LogFile::ERROR, "basf2 stream0-%d was crashed", (int)i);
        setState(RCState::ERROR_ES);
        return;
      }
    }
  }
}

