#include "devNSM2CA.h"

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <recGbl.h>
#include <dbAccess.h>

#include <map>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

namespace Belle2 {
  typedef std::map<std::string, NSMNode> NSMNodeMap;
  typedef std::map<std::string, NSMData> NSMDataMap;
  typedef std::map<std::string, IOSCANPVT*> PVTMap;
}

static Belle2::NSMNodeMap g_node;
static Belle2::NSMDataMap g_data;
static Belle2::PVTMap g_pvt_data;
static Belle2::PVTMap g_pvt_node;
static Belle2::PVTMap g_pvt_req;

namespace Belle2 {

  class NSM2CACallback : public NSMCallback {

  public:
    NSM2CACallback(const NSMNode& node) 
      : NSMCallback(node, 1) { 
    }
    virtual ~NSM2CACallback() throw() {}

  public:
    const RCCommand& getCommand() const { return m_command; }
    void setCommand(const RCCommand& cmd) { m_command = cmd; }

  public:
    virtual void timeout() throw() {
      try {
	NSMCommunicator& com(*getCommunicator());
	for (NSMNodeMap::iterator it = g_node.begin();
	     it != g_node.end(); it++) {
	  NSMNode& node(it->second);
	  if (!com.isConnected(node)) {
	    node.setState(RCState::UNKNOWN);
	    scanIoRequest(*g_pvt_node[it->first]);
	  } else {
	    if (node.getState() == RCState::UNKNOWN) {
	      com.sendRequest(NSMMessage(node, RCCommand::STATECHECK));
	    }
	  }
	}
      } catch (const IOException& e) {
	
      }
      for (NSMDataMap::iterator it = g_data.begin();
	   it != g_data.end(); it++) {
	NSMData& data(it->second);
	if (!data.isAvailable()) {
	  try {
	    data.open(getCommunicator());
	  } catch (const NSMHandlerException& e) {
	    LogFile::error("Failed to open data: %s", it->first.c_str());
	  }
	}
      }
    }

    virtual bool ok() throw() {
      const NSMMessage& msg(getMessage());
      NSMNodeMap::iterator it = g_node.find(msg.getNodeName());
      if (it == g_node.end()) return true;
      std::string name = it->first;
      NSMNode& node(it->second);
      RCState state(msg.getData());
      if (state != RCState::UNKNOWN && node.getState() != state) {
	node.setState(state);
	scanIoRequest(*g_pvt_node[name]);
      }
      return true;
    }
    virtual bool vset(const NSMVar& var) throw() {
      std::string v = var.getText();
      LogFile::debug("vset %s=%s", var.getName().c_str(), v.c_str());
      return true;
    }

  private:
    RCCommand m_command;

  };

}

using namespace Belle2;

static NSM2CACallback* g_callback = NULL;

long init_nsm2(const char* node, const char* host, int port)
{
  if (g_callback == NULL) {
    try {
      NSM2CACallback* callback = new NSM2CACallback(NSMNode(node));
      PThread(new NSMNodeDaemon(callback, host, port));
      g_callback = callback;
    } catch (const IOException& e) {
      LogFile::error("Failed to init NSM2 : %s", e.what());
      return 1;
    }
  }
  return 0;
}

long init_nsm2_longin(longinRecord *record, IOSCANPVT *pvt)
{
  if (record->inp.type != INST_IO) {
    recGblRecordError(S_db_badField, (void*)record, 
		      "init_record Illegal INP field. INT_IO only.");
    return S_db_badField;
  }
  if (record->scan != SCAN_IO_EVENT) {
    recGblRecordError(S_db_badField, (void*)record, 
		      "init_record Illegal SCAN field. IO/INTR scans only.");
    return S_db_badField;
  }
  scanIoInit(pvt);
  record->dpvt = pvt;
  return 0;
}

long init_nsm2_stringin(stringinRecord *record, IOSCANPVT *pvt)
{
  if (record->inp.type != INST_IO) {
    recGblRecordError(S_db_badField, (void*)record, 
		      "init_record Illegal INP field. INT_IO only.");
    return S_db_badField;
  }
  if (record->scan != SCAN_IO_EVENT) {
    recGblRecordError(S_db_badField, (void*)record, 
		      "init_record Illegal SCAN field. IO/INTR scans only.");
    return S_db_badField;
  }
  scanIoInit(pvt);
  record->dpvt = pvt;
  return 0;
}

long init_nsm2_stringout(stringoutRecord *record)
{
  if (record->scan != SCAN_PASSIVE) {
    recGblRecordError(S_db_badField, (void*)record, 
		      "init_record Illegal SCAN field. PASSIVE scans only.");
    return S_db_badField;
  }
  return 0;
}

long get_ioint_info_nsm2_longin(int cmd, longinRecord* record, IOSCANPVT* pvt)
{
  if (cmd != 0) return 0;
  if (!record->dpvt) return 1;
  *pvt = *((IOSCANPVT*)record->dpvt);
  return 0;
}

long get_ioint_info_nsm2_stringin(int cmd, stringinRecord* record, IOSCANPVT* pvt)
{
  if (cmd != 0) return 0;
  if (!record->dpvt) return 1;
  *pvt = *((IOSCANPVT*)record->dpvt);
  return 0;
}

long get_ioint_info_nsm2_data_ai(int cmd, aiRecord* record, IOSCANPVT* pvt)
{
  if (cmd != 0) return 0;
  if (!record->dpvt) return 1;
  *pvt = *((IOSCANPVT*)record->dpvt);
  return 0;
}

long get_ioint_info_nsm2_data_longin(int cmd, longinRecord* record, IOSCANPVT* pvt)
{
  if (cmd != 0) return 0;
  if (!record->dpvt) return 1;
  *pvt = *((IOSCANPVT*)record->dpvt);
  return 0;
}

long read_nsm2_longin(longinRecord *record, int val)
{
  record->val = val;
  record->udf = FALSE;
  return 0;
}

long read_nsm2_ai(aiRecord *record, double val)
{
  record->val = val;
  record->udf = FALSE;
  return 0;
}

long read_nsm2_stringin(stringinRecord *record, const char* val)
{
  memset(record->val, 0, strlen(record->val));
  strcpy(record->val, val);
  record->udf = FALSE;
  return 0;
}

long init_nsm2_data_ai(aiRecord* record)
{
  StringList str = StringUtil::split(record->name, ':');
  if (str.size() >= 3 || str[0] == "nsm2") {
    IOSCANPVT* pvt = new IOSCANPVT;
    std::string name = str[1];
    std::string format = str[2];
    g_data.insert(NSMDataMap::value_type(name, NSMData(name, format, -1)));
    if (record->inp.type != INST_IO) {
      recGblRecordError(S_db_badField, (void*)record, 
			"init_record Illegal INP field. INT_IO only.");
      return S_db_badField;
    }
    if ((record->scan == SCAN_IO_EVENT) || 
      (record->scan == SCAN_PASSIVE)) {
      recGblRecordError(S_db_badField, (void*)record, 
			"Ai (init_record) Illegal SCAN field. Periodic scans only.");
      return S_db_badField;
    }
    scanIoInit(pvt);
    record->dpvt = pvt;
    return 0;
  }
  return 1;
}

long read_nsm2_data_ai(aiRecord* record)
{
  StringList str = StringUtil::split(record->name, ':');
  if (str.size() >= 3 || str[0] == "nsm2") {
    std::string name = str[1];
    NSMData& data(g_data[name]);
    if (!data.isAvailable()) {
      return 1;
    }
    DBField::Type type;
    const void* buf = data.find(StringUtil::join(str, ":", 3), type);
    if (type == DBField::FLOAT) {
      return read_nsm2_ai(record, *(const float*)buf);
    } else if (type == DBField::DOUBLE) {
      return read_nsm2_ai(record, *(const double*)buf);
    }
  }
  return 0;
}

long init_nsm2_data_longin(longinRecord* record)
{
  StringList str = StringUtil::split(record->name, ':');
  if (str.size() >= 3 || str[0] == "nsm2") {
    IOSCANPVT* pvt = new IOSCANPVT;
    std::string name = str[1];
    std::string format = str[2];
    g_data.insert(NSMDataMap::value_type(name, NSMData(name, format, -1)));
    if (record->inp.type != INST_IO) {
      recGblRecordError(S_db_badField, (void*)record, 
			"init_record Illegal INP field. INT_IO only.");
      return S_db_badField;
    }
    if ((record->scan == SCAN_IO_EVENT) || 
      (record->scan == SCAN_PASSIVE)) {
      recGblRecordError(S_db_badField, (void*)record, 
			"Longin (init_record) Illegal SCAN field. Periodic scans only.");
      return S_db_badField;
    }
    scanIoInit(pvt);
    record->dpvt = pvt;
    return 0;
  }
  return 1;
}

long read_nsm2_data_longin(longinRecord* record)
{
  StringList str = StringUtil::split(record->name, ':');
  if (str.size() >= 3 || str[0] == "nsm2") {
    std::string name = str[1];
    NSMData& data(g_data[name]);
    if (!data.isAvailable()) {
      return 1;
    }
    DBField::Type type;
    const void* buf = data.find(StringUtil::join(str, ":", 3), type);
    if (type == DBField::CHAR) {
      return read_nsm2_longin(record, *(const char*)buf);
    } else if (type == DBField::CHAR) {
      return read_nsm2_longin(record, *(const byte8*)buf);
    } else if (type == DBField::SHORT) {
      return read_nsm2_longin(record, *(const int16*)buf);
    } else if (type == DBField::SHORT) {
      return read_nsm2_longin(record, *(const uint16*)buf);
    } else if (type == DBField::INT) {
      return read_nsm2_longin(record, *(const int32*)buf);
    } else if (type == DBField::INT) {
      return read_nsm2_longin(record, *(const uint32*)buf);
    } else if (type == DBField::LONG) {
      return read_nsm2_longin(record, *(const int64*)buf);
    } else if (type == DBField::LONG) {
      return read_nsm2_longin(record, *(const uint64*)buf);
    }
  }
  return 0;
}

long init_nsm2_request_stringin(stringinRecord *record)
{
  StringList str = StringUtil::split(record->name, ':');
  if (str.size() >= 2 || str[0] == "nsm2") {
    IOSCANPVT* pvt = new IOSCANPVT;
    std::string name = str[1];
    g_node.insert(NSMNodeMap::value_type(name, NSMNode(name)));
    return init_nsm2_stringin(record, pvt);
  }
  return 1;
}

long read_nsm2_request_stringin(stringinRecord *record)
{
  StringList str = StringUtil::split(record->name, ':');
  if (str.size() >= 2 || str[0] == "nsm2") {
    std::string name = str[1];
    return read_nsm2_stringin(record, g_node[name].getState().getLabel());
  }
  return 0;
}

long init_nsm2_state_stringin(stringinRecord *record)
{
  StringList str = StringUtil::split(record->name, ':');
  if (str.size() >= 2 || str[0] == "nsm2") {
    std::string name = str[1];
    if (g_node.find(name) == g_node.end()) {
      g_node.insert(NSMNodeMap::value_type(name, NSMNode(name)));
    }
    if (g_pvt_node.find(name) == g_pvt_node.end()) {
      IOSCANPVT* pvt = new IOSCANPVT;
      g_pvt_node.insert(PVTMap::value_type(name, pvt));
      return init_nsm2_stringin(record, pvt);
    }
  }
  return 1;
}

long read_nsm2_state_stringin(stringinRecord *record)
{
  StringList str = StringUtil::split(record->name, ':');
  if (str.size() >= 2 || str[0] == "nsm2") {
    NSMNodeMap::iterator it = g_node.find(str[1]);
    if (it != g_node.end()) {
      NSMNode& node(it->second);
      return read_nsm2_stringin(record, node.getState().getLabel());
    }
  } else {
    LogFile::error("bad PV name : %s", record->name);
  }
  return 1;
}

long init_nsm2_request_stringout(stringoutRecord *record)
{
  StringList str = StringUtil::split(record->name, ':');
  if (str.size() >= 2 || str[0] == "nsm2") {
    std::string name = str[1];
    if (g_node.find(name) == g_node.end()) {
      g_node.insert(NSMNodeMap::value_type(name, NSMNode(name)));
    }
    return init_nsm2_stringout(record);
  }
  return 1;
}

long write_nsm2_request_stringout(stringoutRecord *record)
{
  RCCommand command(record->val);
  if (command != RCCommand::UNKNOWN) {
    StringList str = StringUtil::split(record->name, ':');
    if (str.size() < 2 || str[0] != "nsm2") {
      LogFile::error("Wrong PV name: %s", record->name);
      return 1;
    }
    NSMNodeMap::iterator it = g_node.find(str[1]);
    if (it == g_node.end()) 
      return 1;
    NSMNode& node(it->second);
    try {
      g_callback->getCommunicator()->sendRequest(NSMMessage(node, command));
    } catch (const NSMHandlerException& e) {
      LogFile::error("Failed to reply to NSM : %s", e.what());
      return 2;
    }
    RCState tstate(command.nextTState());
    if (tstate != Enum::UNKNOWN) {
      node.setState(tstate);
      scanIoRequest(*g_pvt_node[it->first]);
    }
    return 0;
  } else {
    LogFile::error("Unknown state : %s", record->val);
  }
  return 1;
}

