#include "devRC.h"

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <recGbl.h>
#include <dbAccess.h>

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

static IOSCANPVT* g_pvt_expno = new IOSCANPVT;
static IOSCANPVT* g_pvt_runno = new IOSCANPVT;
static IOSCANPVT* g_pvt_subno = new IOSCANPVT;
static IOSCANPVT* g_pvt_configid = new IOSCANPVT;
static IOSCANPVT* g_pvt_request = new IOSCANPVT;

namespace Belle2 {

  class RCEpicsCallback : public RCCallback {

  public:
    RCEpicsCallback(const NSMNode& node) 
      : RCCallback(node) { setAutoReply(false); }
    virtual ~RCEpicsCallback() throw() {}

  public:
    const RCCommand& getCommand() const { return m_command; }
    void setCommand(const RCCommand& cmd) { m_command = cmd; }

  public:
    virtual bool load() throw() {
      scanIoRequest(*g_pvt_configid);
      return setCommand();
    }
    virtual bool start() throw() {
      scanIoRequest(*g_pvt_expno);
      scanIoRequest(*g_pvt_runno);
      scanIoRequest(*g_pvt_subno);
      return setCommand();
    }
    virtual bool stop() throw() { return setCommand(); }
    virtual bool resume() throw() { return setCommand(); }
    virtual bool pause() throw() { return setCommand(); }
    virtual bool recover() throw() { return setCommand(); }
    virtual bool abort() throw() { return setCommand(); }

  private:
    bool setCommand() {
      m_command = getMessage().getRequestName();
      scanIoRequest(*g_pvt_request);
      return true;
    }

  private:
    RCCommand m_command;

  };

}

using namespace Belle2;

static RCEpicsCallback* g_callback = NULL;

long init_rc(const char* node, const char* host, int port)
{
  if (g_callback == NULL) {
    try {
      RCEpicsCallback* callback = new RCEpicsCallback(NSMNode(node));
      PThread(new NSMNodeDaemon(callback, host, port));
      g_callback = callback;
    } catch (const IOException& e) {
      LogFile::error("Failed to init NSM2 : %s", e.what());
      return 1;
    }
  }
  return 0;
}

long init_rc_longin(longinRecord *record, IOSCANPVT *pvt)
{
  if (record->inp.type != INST_IO) {
    recGblRecordError(S_db_badField, (void*)record, 
		      "Waveform (init_record) Illegal INP field. INT_IO only.");
    return S_db_badField;
  }
  if (record->scan != SCAN_IO_EVENT) {
    recGblRecordError(S_db_badField, (void*)record, 
		      "Waveform (init_record) Illegal SCAN field. IO/INTR scans only.");
    return S_db_badField;
  }
  scanIoInit(pvt);
  record->dpvt = pvt;
  return 0;
}

long init_rc_stringin(stringinRecord *record, IOSCANPVT *pvt)
{
  if (record->inp.type != INST_IO) {
    recGblRecordError(S_db_badField, (void*)record, 
		      "Waveform (init_record) Illegal INP field. INT_IO only.");
    return S_db_badField;
  }
  if (record->scan != SCAN_IO_EVENT) {
    recGblRecordError(S_db_badField, (void*)record, 
		      "Waveform (init_record) Illegal SCAN field. IO/INTR scans only.");
    return S_db_badField;
  }
  scanIoInit(pvt);
  record->dpvt = pvt;
  return 0;
}

long init_rc_stringout(stringoutRecord *record)
{
  if (record->scan != SCAN_PASSIVE) {
    recGblRecordError(S_db_badField, (void*)record, 
		      "Waveform (init_record) Illegal SCAN field. PASSIVE scans only.");
    return S_db_badField;
  }
  return 0;
}

long get_ioint_info_rc_longin(int cmd, longinRecord* record, IOSCANPVT* pvt)
{
  if (cmd != 0) return 0;
  if (!record->dpvt) return 1;
  *pvt = *((IOSCANPVT*)record->dpvt);
  return 0;
}

long get_ioint_info_rc_stringin(int cmd, stringinRecord* record, IOSCANPVT* pvt)
{
  if (cmd != 0) return 0;
  if (!record->dpvt) return 1;
  *pvt = *((IOSCANPVT*)record->dpvt);
  return 0;
}

long read_rc_longin(longinRecord *record, int val)
{
  record->val = val;
  record->udf = FALSE;
  return 0;
}

long read_rc_stringin(stringinRecord *record, const char* val)
{
  memset(record->val, 0, strlen(record->val));
  strcpy(record->val, val);
  record->udf = FALSE;
  return 0;
}

long init_rc_expno_longin(longinRecord *record)
{
  return init_rc_longin(record, g_pvt_expno);
}

long read_rc_expno_longin(longinRecord *record)
{
  return read_rc_longin(record, g_callback->getConfig().getExpNumber());
}

long init_rc_runno_longin(longinRecord *record)
{
  return init_rc_longin(record, g_pvt_runno);
}

long read_rc_runno_longin(longinRecord *record)
{
  return read_rc_longin(record, g_callback->getConfig().getRunNumber());
}

long init_rc_subno_longin(longinRecord *record)
{
  return init_rc_longin(record, g_pvt_subno);
}

long read_rc_subno_longin(longinRecord *record)
{
  return read_rc_longin(record, g_callback->getConfig().getSubNumber());
}

long init_rc_configid_longin(longinRecord *record)
{
  return init_rc_longin(record, g_pvt_configid);
}

long read_rc_configid_longin(longinRecord *record)
{
  return read_rc_longin(record, g_callback->getConfig().getConfigId());
}

long init_rc_request_stringin(stringinRecord *record)
{
  return init_rc_stringin(record, g_pvt_request);
}

long read_rc_request_stringin(stringinRecord *record)
{
  return read_rc_stringin(record, g_callback->getCommand().getLabel());
}

long init_rc_state_stringout(stringoutRecord *record)
{
  std::cout << record->name << "=" << record->val << std::endl;
  return init_rc_stringout(record);
}

long write_rc_state_stringout(stringoutRecord *record)
{
  RCState state(record->val);
  if (state != RCState::UNKNOWN) {
    NSMNode& node(g_callback->getNode());
    node.setState(state);
    try {
      if (state.isStable()) {
	g_callback->setCommand(RCCommand::UNKNOWN);
	scanIoRequest(*g_pvt_request);
      }
      g_callback->getCommunicator()->replyOK(node);
    } catch (const NSMHandlerException& e) {
      LogFile::error("Failed to reply to NSM : %s", e.what());
      return 2;
    }
    return 0;
  } else {
    LogFile::error("Unknown state : %s", record->val);
    return 1;
  }
}

