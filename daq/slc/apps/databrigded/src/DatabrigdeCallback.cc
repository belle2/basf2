#include "daq/slc/apps/databrigded/DatabrigdeCallback.h"

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

DatabrigdeCallback::DatabrigdeCallback()
{
  m_con.setCallback(this);
  m_showall = false;
}

DatabrigdeCallback::~DatabrigdeCallback() throw()
{
}

/*
   initialize() : Initialize with parameters from configuration database.
                  Called at the beginings of NSM node.
   obj          : Contains parameters in configuration from the database.
*/
void DatabrigdeCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  m_con.init("databrigde", 1);
  add(new NSMVHandlerInt("svd.used", true, true, 1));
  add(new NSMVHandlerInt("ttd.used", true, true, 1));
}

/*
   configure() : Read parameters from configuration database.
                 Called by RC_CONFIGURE at NOTREADY.
   obj         : Contains parameters in configuration from the database.
*/
void DatabrigdeCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
}

/*
   monitor() : Check status at each end of wainting time limit.
*/
void DatabrigdeCallback::monitor() throw(RCHandlerException)
{
}

/*
   load() : Perform parameter download etc to READY.
   obj    : Contains configuration read by the last initailize or configure.
            Values in obj might be modified by nsmvset.
*/
void DatabrigdeCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  if (m_con.isAlive()) return;
  int svd_used = 0, ttd_used = 0;
  get("svd.used", svd_used);
  get("ttd.used", ttd_used);
  std::string host = "127.0.0.1";
  int port = 5121;
  std::string script;
  try {
    m_con.clearArguments();
    m_con.setExecutable("/home/usr/b2daq/eb/eb1rx");
    m_con.addArgument("-l");
    m_con.addArgument("%d", 5121);
    int nsenders = 0;
    if (svd_used) {
      m_con.addArgument("tx-rpc1:5101");
      nsenders++;
    }
    if (ttd_used) {
      m_con.addArgument("-F");
      m_con.addArgument("tx-ttd10:30000");
      nsenders++;
    }
    std::string upname = std::string("/dev/shm/") + getNode().getName() + "_eb1rx_up";
    std::string downname = std::string("/dev/shm/") + getNode().getName() + "_eb1rx_down";
    if (m_eb_stat) delete m_eb_stat;
    LogFile::debug("wcreating eb_statistics(%s, %d, %s, %d)", upname.c_str(), nsenders, downname.c_str(), 1);
    m_eb_stat = new eb_statistics(upname.c_str(), nsenders, downname.c_str(), 1);
    m_con.addArgument("-u");
    m_con.addArgument(upname);
    m_con.addArgument("-d");
    m_con.addArgument(downname);
  } catch (const std::out_of_range& e) {
    throw (RCHandlerException(e.what()));
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
void DatabrigdeCallback::start(int expno, int runno) throw(RCHandlerException)
{
  LogFile::debug("run # = %04d.%04d.%03d", expno, runno, 0);
  LogFile::debug("Start done");
}

/*
   stop() : End run by stopping triggers to READY
*/
void DatabrigdeCallback::stop() throw(RCHandlerException)
{
  LogFile::debug("Stop done");
}

/*
   resume() : Restart triggers back to RUNNING
   subno    : Sub run number for the resumed run
   returns true on success or false on failed
*/
bool DatabrigdeCallback::resume(int subno) throw(RCHandlerException)
{
  LogFile::debug("sub run # = %03d", subno);
  LogFile::debug("Resume done");
  return true;
}

/*
   pause() : Suspend triggers to PAUSED
   returns true on success or false on failed
*/
bool DatabrigdeCallback::pause() throw(RCHandlerException)
{
  LogFile::debug("Pause done");
  return true;
}

/*
   recover() : Recover errors to be back to READY
*/
void DatabrigdeCallback::recover(const DBObject& obj) throw(RCHandlerException)
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
void DatabrigdeCallback::abort() throw(RCHandlerException)
{
  m_con.abort();
  LogFile::debug("Abort done");
}

