#include "daq/slc/apps/eb1rxd/EB1RXCallback.h"
#include "daq/slc/apps/eb1rxd/eb1rx_status.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

using namespace Belle2;

EB1RXCallback::EB1RXCallback()
{
  m_con.setCallback(this);
  m_showall = false;
  m_nsenders = 0;
}

EB1RXCallback::~EB1RXCallback() throw()
{
}

/*
   initialize() : Initialize with parameters from configuration database.
                  Called at the beginings of NSM node.
   obj          : Contains parameters in configuration from the database.
*/
void EB1RXCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  allocData(getNode().getName(), "eb1rx", eb1rx_status_revision);
  m_con.init("eb1rx", 1);
  const DBObjectList& o_txs(obj.getObjects("tx"));
  for (size_t i = 0; i < o_txs.size(); i++) {
    const DBObject& o_tx(o_txs[i]);
    bool used = o_tx.getBool("used");
    std::string vname = o_tx.getText("name") + ".";
    std::string rname = StringUtil::form("tx[%d].", m_nsenders);
    add(new NSMVHandlerRef(*this, vname + "used", rname + "used"));
    add(new NSMVHandlerRef(*this, vname + "connection", rname + "connection"));
    if (used) {
      std::string vname = StringUtil::form("stat.in[%d].", m_nsenders);
      add(new NSMVHandlerInt(vname + "addr", true, false, 0));
      add(new NSMVHandlerInt(vname + "port", true, false, 0));
      add(new NSMVHandlerInt(vname + "connection", true, false, 0));
      add(new NSMVHandlerInt(vname + "byte", true, false, 0));
      add(new NSMVHandlerInt(vname + "event", true, false, 0));
      add(new NSMVHandlerFloat(vname + "total_byte", true, false, 0));
      //add(new NSMVHandlerFloat(vname + "nevent", true, false, 0));
      //add(new NSMVHandlerFloat(vname + "evtrate", true, false, 0));
      add(new NSMVHandlerFloat(vname + "flowrate", true, false, 0));
      m_nsenders++;
    }
  }
  std::string vname = StringUtil::form("stat.out.");
  add(new NSMVHandlerInt(vname + "addr", true, false, 0));
  add(new NSMVHandlerInt(vname + "port", true, false, 0));
  add(new NSMVHandlerInt(vname + "connection", true, false, 0));
  add(new NSMVHandlerInt(vname + "byte", true, false, 0));
  add(new NSMVHandlerInt(vname + "event", true, false, 0));
  add(new NSMVHandlerFloat(vname + "total_byte", true, false, 0));
  //add(new NSMVHandlerFloat(vname + "nevent", true, false, 0));
  //add(new NSMVHandlerFloat(vname + "evtrate", true, false, 0));
  add(new NSMVHandlerFloat(vname + "flowrate", true, false, 0));
  std::string upname = std::string("/dev/shm/") + getNode().getName() + "_eb1rx_up";
  std::string downname = std::string("/dev/shm/") + getNode().getName() + "_eb1rx_down";
  if (m_eb_stat) delete m_eb_stat;
  LogFile::debug("wcreating eb_statistics(%s, %d, %s, %d)", upname.c_str(), m_nsenders, downname.c_str(), 1);
  m_eb_stat = new eb_statistics(upname.c_str(), m_nsenders, downname.c_str(), 1);
  configure(obj);
}

/*
   configure() : Read parameters from configuration database.
                 Called by RC_CONFIGURE at NOTREADY.
   obj         : Contains parameters in configuration from the database.
*/
void EB1RXCallback::configure(const DBObject& /*obj*/) throw(RCHandlerException)
{
}

/*
   load() : Perform parameter download etc to READY.
   obj    : Contains configuration read by the last initailize or configure.
            Values in obj might be modified by nsmvset.
*/
void EB1RXCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  if (m_con.isAlive()) return;
  try {
    int port = obj.getInt("port");
    std::string exe = obj.getText("executable");
    std::string script;
    const DBObjectList& o_txs(obj.getObjects("tx"));
    m_con.clearArguments();
    m_con.setExecutable(exe);
    m_con.addArgument("-l");
    m_con.addArgument("%d", port);
    m_nsenders = 0;
    for (size_t i = 0; i < o_txs.size(); i++) {
      const DBObject& o_tx(o_txs[i]);
      bool used = o_tx.getBool("used");
      if (!used) continue;
      bool isftsw = o_tx.getBool("ftsw");
      if (isftsw) m_con.addArgument("-F");
      std::string host = o_tx.getText("host");
      int port = o_tx.getInt("port");
      m_con.addArgument("%s:%d", host.c_str(), port);
      m_nsenders++;
    }
    std::string upname = std::string("/dev/shm/") + getNode().getName() + "_eb1rx_up";
    std::string downname = std::string("/dev/shm/") + getNode().getName() + "_eb1rx_down";
    m_con.addArgument("-u");
    m_con.addArgument(upname);
    m_con.addArgument("-d");
    m_con.addArgument(downname);
  } catch (const std::out_of_range& e1) {
    throw (RCHandlerException(e1.what()));
  }
  try {
    m_con.load(0);
  } catch (const std::exception& e) {
    LogFile::warning("load timeout");
  }
  LogFile::debug("Load done");

}

/*
   start() : Start triggers to RUNNING
   expno   : Experiment number from RC parent
   runno   : Run number from RC parent
*/
void EB1RXCallback::start(int expno, int runno) throw(RCHandlerException)
{
  LogFile::debug("run # = %04d.%04d.%03d", expno, runno, 0);
  LogFile::debug("Start done");
}

/*
   stop() : End run by stopping triggers to READY
*/
void EB1RXCallback::stop() throw(RCHandlerException)
{
  LogFile::debug("Stop done");
}

/*
   resume() : Restart triggers back to RUNNING
   subno    : Sub run number for the resumed run
   returns true on success or false on failed
*/
bool EB1RXCallback::resume(int subno) throw(RCHandlerException)
{
  LogFile::debug("sub run # = %03d", subno);
  LogFile::debug("Resume done");
  return true;
}

/*
   pause() : Suspend triggers to PAUSED
   returns true on success or false on failed
*/
bool EB1RXCallback::pause() throw(RCHandlerException)
{
  LogFile::debug("Pause done");
  return true;
}

/*
   recover() : Recover errors to be back to READY
*/
void EB1RXCallback::recover(const DBObject& obj) throw(RCHandlerException)
{
  // abort to be NOTREADY
  abort();
  // load to be READY
  load(obj);
  LogFile::debug("Recover done");
}

/*
   abort() : Discard all configuration to back to NOTREADY
*/
void EB1RXCallback::abort() throw(RCHandlerException)
{
  m_con.abort();
  LogFile::debug("Abort done");
}

/*
   monitor() : Check status at each end of wainting time limit.
*/
void EB1RXCallback::monitor() throw(RCHandlerException)
{
  double t1 = Time().get();
  double dt = t1 - m_t0;
  const RCState state(getNode().getState());
  if (state == RCState::RUNNING_S || state == RCState::READY_S ||
      state == RCState::PAUSED_S || state == RCState::LOADING_TS ||
      state == RCState::STARTING_TS) {
    if (!m_con.isAlive()) {
      setState(RCState::ERROR_ES);
      throw (RCHandlerException(m_con.getParName() + " : crashed"));
    }
  }
  if (state == RCState::RUNNING_S || state == RCState::READY_S) {
    std::string vname = StringUtil::form("stat.out.");
    set(vname + "event", (int)m_eb_stat->down(0).event);
    set(vname + "byte", (float)(m_eb_stat->down(0).byte / 1024.));
    set(vname + "addr", (int)m_eb_stat->down(0).event);
    set(vname + "port", (int)m_eb_stat->down(0).port);
    set(vname + "connection", (int)m_eb_stat->down(0).port > 0);
    double total_byte = m_eb_stat->down(0).total_byte;
    double flowrate = (m_total_byte_out[0] - total_byte) / dt;
    set(vname + "total_byte", (float)total_byte);
    set(vname + "floarate", (float)flowrate);
    m_total_byte_out[0] = total_byte;
    for (int i = 0; i < m_nsenders; i++) {
      std::string vname = StringUtil::form("stat.in[%d].", i);
      set(vname + "event", (int)m_eb_stat->up(i).event);
      set(vname + "byte", (float)(m_eb_stat->up(i).byte / 1024.));
      set(vname + "addr", (int)m_eb_stat->up(i).event);
      set(vname + "port", (int)m_eb_stat->up(i).port);
      set(vname + "connection", (int)m_eb_stat->up(i).port > 0);
      double total_byte = m_eb_stat->down(0).total_byte;
      double flowrate = (m_total_byte_in[i] - total_byte) / dt;
      set(vname + "total_byte", (float)total_byte);
      set(vname + "floarate", (float)flowrate);
      m_total_byte_in[i] = total_byte;
    }
  } else {
    std::string vname = StringUtil::form("stat.out.");
    set(vname + "event", 0);
    set(vname + "byte", (float)0);
    set(vname + "addr", 0);
    set(vname + "port", 0);
    set(vname + "connection", 0);
    set(vname + "total_byte", 0);
    set(vname + "floarate", 0);
    for (int i = 0; i < m_nsenders; i++) {
      std::string vname = StringUtil::form("stat.in[%d].", i);
      set(vname + "event", 0);
      set(vname + "byte", 0);
      set(vname + "addr", 0);
      set(vname + "port", 0);
      set(vname + "connection", 0);
      set(vname + "total_byte", 0);
      set(vname + "floarate", 0);
    }
  }
  m_t0 = t1;
}
