#include "daq/slc/apps/eb1rxd/Eb1rxCallback.h"

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

Eb1rxCallback::Eb1rxCallback()
{
}

Eb1rxCallback::~Eb1rxCallback() throw()
{
}

/*
   initialize() : Initialize with parameters from configuration database.
                  Called at the beginings of NSM node.
   obj          : Contains parameters in configuration from the database.
*/
void Eb1rxCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  configure(obj);
}

/*
   configure() : Read parameters from configuration database.
                 Called by RC_CONFIGURE at NOTREADY.
   obj         : Contains parameters in configuration from the database.
*/
void Eb1rxCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  // check if an object "value" exsits
  try {
    const DBObjectList& o_txs(obj.getObjects("tx"));
    add(new NSMVHandlerInt("ntxs", true, false, o_txs.size()));
  } catch (const std::out_of_range& e) {
    throw (RCHandlerException(e.what()));
  }
  m_con.setCallback(this);
  m_con.init("eb1rx", 0);
  m_flow.open(&m_con.getInfo());
}

/*
   monitor() : Check status at each end of wainting time limit.
*/
void Eb1rxCallback::monitor() throw(RCHandlerException)
{
  const RCState state(getNode().getState());
  if (state == RCState::RUNNING_S || state == RCState::READY_S ||
      state == RCState::PAUSED_S || state == RCState::LOADING_TS ||
      state == RCState::STARTING_TS) {
    if (!m_con.isAlive()) {
      setState(RCState::NOTREADY_S);
      throw (RCHandlerException(m_con.getName() + " : crashed"));
    }
  }
}

/*
   load() : Perform parameter download etc to READY.
   obj    : Contains configuration read by the last initailize or configure.
            Values in obj might be modified by nsmvset.
*/
void Eb1rxCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  if (m_con.isAlive()) {
    LogFile::debug("process already booted (pid=%d)",
                   m_con.getProcess().get_id());
    return;
  }
  std::string executable = obj.getText("executable");
  int port = obj.getInt("port");
  m_con.clearArguments();
  m_con.setExecutable(executable);
  m_con.addArgument("-l");
  m_con.addArgument(port);
  const DBObjectList& o_txs(obj.getObjects("tx"));
  for (DBObjectList::const_iterator i = o_txs.begin();
       i != o_txs.end(); i++) {
    const DBObject& o_tx(*i);
    const int port = o_tx.getInt("port");
    const std::string host = o_tx.getText("host");
    if (o_tx.getInt("used")) {
      m_con.addArgument("%s:%d", host.c_str(), port);
    }
  }
  try {
    m_con.load(-1);
  } catch (const std::exception& e) {
    throw (RCHandlerException(e.what()));
  }
  LogFile::debug("Load done");
}

/*
   start() : Start triggers to RUNNING
   expno   : Experiment number from RC parent
   runno   : Run number from RC parent
*/
void Eb1rxCallback::start(int expno, int runno) throw(RCHandlerException)
{
  LogFile::debug("run # = %04d.%04d.%03d", expno, runno, 0);
  LogFile::debug("Start done");
}

/*
   stop() : End run by stopping triggers to READY
*/
void Eb1rxCallback::stop() throw(RCHandlerException)
{
  LogFile::debug("Stop done");
}

/*
   resume() : Restart triggers back to RUNNING
   subno    : Sub run number for the resumed run
*/
bool Eb1rxCallback::resume(int subno) throw(RCHandlerException)
{
  LogFile::debug("sub run # = %03d", subno);
  LogFile::debug("Resume done");
  return true;
}

/*
   pause() : Suspend triggers to PAUSED
*/
bool Eb1rxCallback::pause() throw(RCHandlerException)
{
  LogFile::debug("Pause done");
  return true;
}

/*
   recover() : Recover errors to be back to READY
*/
void Eb1rxCallback::recover(const DBObject& obj) throw(RCHandlerException)
{
  // load to be READY
  load(obj);
  LogFile::debug("Recover done");
}

/*
   abort() : Discard all configuration to back to NOTREADY
*/
void Eb1rxCallback::abort() throw(RCHandlerException)
{
  m_con.abort();
  LogFile::debug("Abort done");
}

