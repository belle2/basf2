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

#include <unistd.h>

namespace Belle2 {

  class NSMVHandlerROPID : public NSMVHandlerInt {
  public:
    NSMVHandlerROPID(ROController& con, const std::string& name)
      : NSMVHandlerInt(name, true, false), m_con(con) {}
    virtual ~NSMVHandlerROPID() throw() {}
    bool handleGetInt(int& val)
    {
      val = m_con.getControl().getProcess().get_id();
      return true;
    }
  private:
    ROController& m_con;
  };

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

}

using namespace Belle2;

ROCallback::ROCallback(const NSMNode& runcontrol)
  : RCCallback(), m_runcontrol(runcontrol)
{
  system("killall basf2");
}

void ROCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  allocData(getNode().getName(), "ropc", ropc_revision);
  configure(obj);
}

void ROCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  try {
    const DBObjectList& stream0(obj.getObjects("stream0"));
    m_eb0.init(this, 0, "eb0", obj);
    if (obj.hasObject("stream1")) {
      m_stream1.init(this, 1, "stream1", obj);
      add(new NSMVHandlerROInputPort(m_stream1, "stream1.input.port"));
      add(new NSMVHandlerROOutputPort(m_stream1, "stream1.output.port"));
    }
    m_stream0 = std::vector<Stream0Controller>();
    for (size_t i = 0; i < stream0.size(); i++) {
      m_stream0.push_back(Stream0Controller());
    }
    for (size_t i = 0; i < m_stream0.size(); i++) {
      std::string vname = (m_stream0.size() == 1) ? "stream0" : StringUtil::form("stream0[%d]", (int)i);
      m_stream0[i].init(this, i + 2, vname, obj);
      add(new NSMVHandlerROInputPort(m_stream0[i], vname + ".input.port"));
      add(new NSMVHandlerROOutputPort(m_stream0[i], vname + ".output.port"));
    }
    //m_eb1tx.init(this, 0, "eb1tx", obj);
  } catch (const std::out_of_range& e) {
    throw (RCHandlerException(e.what()));
  }
}

void ROCallback::term() throw()
{
  m_stream1.term();
  m_eb0.term();
  for (size_t i = 0; i < m_stream0.size(); i++) {
    m_stream0[0].term();
  }
  //m_eb1tx.term();
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
  /*
  if (obj.hasObject("stream1")) {
    if (!m_stream1.load(obj, 10)) {
      throw (RCHandlerException("Faield to boot stream1"));
    }
  }
  if (!m_eb1tx.load(obj, 0)) {
    throw (RCHandlerException("Faield to boot eb1tx"));
  }
  */
  LogFile::debug("Booted stream1");
  try_wait();
}

void ROCallback::start(int /*expno*/, int /*runno*/) throw(RCHandlerException)
{
  /*
  try {
    if (m_eb0.isUsed()) m_eb0.start(expno, runno);
  } catch (const RCHandlerException& e) {
    LogFile::warning("eb0 did not start : %s", e.what());
  }
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
  //m_eb1tx.start(expno, runno);
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
  m_stream1.stop();
  for (size_t i = 0; i < m_stream0.size(); i++) {
    m_stream0[i].stop();
  }
  m_eb0.stop();
}

void ROCallback::recover(const DBObject& obj) throw(RCHandlerException)
{
  //abort();
  load(obj);
}

void ROCallback::abort() throw(RCHandlerException)
{
  m_stream1.abort();
  for (size_t i = 0; i < m_stream0.size(); i++) {
    m_stream0[i].abort();
  }
  m_eb0.abort();
  //m_eb1tx.abort();
  //set("eb1tx.pid", -1);
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
    } else {
      nsm->loadavg = -1;
    }
    data.flush();
  }
  const RCState state(getNode().getState());
  if (state == RCState::RUNNING_S || state == RCState::LOADING_TS ||
      state == RCState::STARTING_TS) {
    if (m_eb0.isUsed() && !m_eb0.getControl().isAlive()) {
      setState(RCState::NOTREADY_S);
      log(LogFile::ERROR, "eb0 was crashed");
    }
    for (size_t i = 0; i < m_stream0.size(); i++) {
      if (m_stream0[i].isUsed() && !m_stream0[i].getControl().isAlive()) {
        setState(RCState::NOTREADY_S);
        log(LogFile::ERROR, "basf2 stream0-%d was crashed", (int)i);
      }
    }
  }
}

