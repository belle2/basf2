#include "daq/slc/apps/rctemplated/RCTemplateCallback.h"

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

RCTemplateCallback::RCTemplateCallback()
{
  m_showall = false;
}

RCTemplateCallback::~RCTemplateCallback() throw()
{
}

/*
   initialize() : Initialize with parameters from configuration database.
                  Called at the beginings of NSM node.
   obj          : Contains parameters in configuration from the database.
*/
void RCTemplateCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
}

/*
   configure() : Read parameters from configuration database.
                 Called by RC_CONFIGURE at NOTREADY.
   obj         : Contains parameters in configuration from the database.
*/
void RCTemplateCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  /*
  // check if an object "value" exsits
  if (obj.hasObject("value")) {
    // get object "value"
    const DBObject& o_value(obj("value"));
    // check if a number "ival" exsits
    if (o_value.hasValue("ival")) {
      LogFile::debug("ival=%d", o_value.getInt("ival"));
    }
    // check if a number "bval" exsits
    if (o_value.hasValue("bval")) {
      LogFile::debug("bval=%s", (o_value.getBool("bval") ? "true" : "false"));
    }
    // check if a number "fval" exsits
    if (o_value.hasValue("fval")) {
      LogFile::debug("fval=%f", o_value.getFloat("fval"));
    }
    // check if a text "ival" exsits
    if (o_value.hasText("text")) {
      LogFile::debug("text=%s", o_value.getText("text").c_str());
    }
  }
  */
}

/*
   monitor() : Check status at each end of wainting time limit.
*/
void RCTemplateCallback::monitor() throw(RCHandlerException)
{
}

/*
   load() : Perform parameter download etc to READY.
   obj    : Contains configuration read by the last initailize or configure.
            Values in obj might be modified by nsmvset.
*/
void RCTemplateCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  LogFile::debug("Load done");
}

/*
   start() : Start triggers to RUNNING
   expno   : Experiment number from RC parent
   runno   : Run number from RC parent
*/
void RCTemplateCallback::start(int expno, int runno) throw(RCHandlerException)
{
  LogFile::debug("run # = %04d.%04d.%03d", expno, runno, 0);
  LogFile::debug("Start done");
}

/*
   stop() : End run by stopping triggers to READY
*/
void RCTemplateCallback::stop() throw(RCHandlerException)
{
  LogFile::debug("Stop done");
}

/*
   resume() : Restart triggers back to RUNNING
   subno    : Sub run number for the resumed run
   returns true on success or false on failed
*/
bool RCTemplateCallback::resume(int subno) throw(RCHandlerException)
{
  LogFile::debug("sub run # = %03d", subno);
  LogFile::debug("Resume done");
  return true;
}

/*
   pause() : Suspend triggers to PAUSED
   returns true on success or false on failed
*/
bool RCTemplateCallback::pause() throw(RCHandlerException)
{
  LogFile::debug("Pause done");
  return true;
}

/*
   recover() : Recover errors to be back to READY
*/
void RCTemplateCallback::recover(const DBObject& obj) throw(RCHandlerException)
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
void RCTemplateCallback::abort() throw(RCHandlerException)
{
  LogFile::debug("Abort done");
}

