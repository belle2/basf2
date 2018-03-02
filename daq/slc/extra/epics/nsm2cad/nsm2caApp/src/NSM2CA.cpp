#include "NSM2CA.h"

#include <dbScan.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>

using namespace Belle2;

IOSCANPVT* NSM2CA::init_vget_in(const char* recordname)
{
  std::string node, vname;
  if (find(recordname, "get", node, vname)) {
    node = StringUtil::toupper(node);
    var_pvt var = {NSMVar(), new IOSCANPVT};
    try {
      m_var.insert(std::map<std::string, var_pvt>::value_type(node+"."+vname, var));
      NSMCommunicator::send(NSMMessage(NSMNode(node), NSMCommand::VGET, vname));
    } catch (const NSMHandlerException& e) {
      LogFile::error("%s.%d : ", __FILE__, __LINE__, e.what());
    } catch (const TimeoutException& e) {
    }
    return var.pvt;
  }
  return NULL;
}

long NSM2CA::read_vget_in(aiRecord* record)
{
  std::string node, vname;
  if (find(record->name, "get", node, vname)) {
    node = StringUtil::toupper(node);
    try {
      float val = getFloat(node + "." + vname);
      if (m_orecords.find(vname) != m_orecords.end()) {
	aoRecord* orecord = (aoRecord*)m_orecords[vname];
	orecord->val = val;
	orecord->udf = FALSE;
      }
      std::string name = node + "." + vname;
      record->val = val;
      record->udf = FALSE;
    } catch (const std::exception& e) {
      LogFile::error(e.what());
    }
  }
  return 2;
}

long NSM2CA::read_vget_in(longinRecord* record)
{
  std::string node, vname;
  if (find(record->name, "get", node, vname)) {
    node = StringUtil::toupper(node);
    try {
      int val = getInt(node + "." + vname);
      if (m_orecords.find(vname) != m_orecords.end()) {
	longoutRecord* orecord = (longoutRecord*)m_orecords[vname];
	orecord->val = val;
	orecord->udf = FALSE;
      }
      record->val = val;
      record->udf = FALSE;
      return 0;
    } catch (const std::exception& e) {
      LogFile::error(e.what());
    }
  }
  return 2;
}

long NSM2CA::read_vget_in(stringinRecord* record)
{
  std::string node, vname;
  if (find(record->name, "get", node, vname)) {
    node = StringUtil::toupper(node);
    try {
      std::string val = getText(node + "." + vname);
      if (m_orecords.find(vname) != m_orecords.end()) {
	stringoutRecord* orecord = (stringoutRecord*)m_orecords[vname];
	orecord->val[0] = 0;
	strcpy(orecord->val, val.c_str());
	orecord->udf = FALSE;
      }
      memset(record->val, 0, strlen(record->val));
      strcpy(record->val, val.c_str());
      record->udf = FALSE;
    } catch (const std::exception& e) {
      LogFile::error(e.what());
    }
  }
  return 0;
}

bool NSM2CA::init_vset_out(const char* recordname, void* record)
{
  std::string nodename, vname;
  if (find(recordname, "set", nodename, vname)) {
    m_orecords.insert(std::map<std::string, void*>::value_type(vname, record));
    return true;
  }
  return false;
}

long NSM2CA::write_vset_out(aoRecord* record)
{
  std::string nodename, vname;
  if (find(record->name, "set", nodename, vname)) {
    nodename = StringUtil::toupper(nodename);
    NSMNode node(nodename);
    NSMMessage msg(node, NSMVar(vname, (float)record->val));
    try {
      return wait(5, msg, nodename + "." + vname).getParam(0) == 1;
    } catch (const TimeoutException& e) {
      LogFile::error("%s.%d : %s %s", __FILE__, __LINE__, e.what(), record->name);
      return 1;
    } catch (const IOException& e) {
      LogFile::warning(e.what());
      return 1;
    }
    return 0;
  }
  return 1;
}

long NSM2CA::write_vset_out(longoutRecord* record)
{
  std::string nodename, vname;
  if (find(record->name, "set", nodename, vname)) {
    nodename = StringUtil::toupper(nodename);
    NSMNode node(nodename);
    NSMMessage msg(node, NSMVar(vname, (int)record->val));
    try {
      return wait(5, msg, nodename + "." + vname).getParam(0) == 1;
      } catch (const TimeoutException& e) {
      LogFile::error("%s.%d : %s %s", __FILE__, __LINE__, e.what(), record->name);
    }
    return 0;
  }
  return 1;
}

long NSM2CA::write_vset_out(stringoutRecord* record)
{
  std::string nodename, vname;
  if (find(record->name, "set", nodename, vname)) {
    nodename = StringUtil::toupper(nodename);
    NSMNode node(nodename);
    if (vname == "rcrequest") {
      LogFile::info("rcrequest : %s", record->val);
      std::string s = StringUtil::toupper(StringUtil::replace(record->val, ":", "_"));
      if (!StringUtil::find(s, "RC_")) s = "RC_" + s;
      RCCommand command(s);
      if (command != RCCommand::UNKNOWN) {
	NSMCommunicator::send(NSMMessage(node, command));
      }
    } else if (vname == "rcconfig") {
      NSMCommunicator::send(NSMMessage(node, RCCommand::CONFIGURE, record->val));
    } else if (vname == "hvrequest") {
      LogFile::info("hvrequest : %s", record->val);
      std::string s = StringUtil::toupper(StringUtil::replace(record->val, ":", "_"));
      if (!StringUtil::find(s, "HV_")) s = "HV_" + s;
      HVCommand command(s);
      if (command != RCCommand::UNKNOWN) {
	NSMCommunicator::send(NSMMessage(node, command));
      }
    } else if (vname == "hvconfig") {
      NSMCommunicator::send(NSMMessage(node, HVCommand::CONFIGURE, record->val));
    } else {
      try {
	NSMMessage msg(node, NSMVar(vname, record->val));
	return wait(5, msg, nodename + "." + vname).getParam(0) == 1;
      } catch (const IOException& e) {
	LogFile::error("%s.%d : %s %s", __FILE__, __LINE__, e.what(), record->name);
      }
    }
    return 0;
  } else {
    LogFile::error("error : %s set %s %s", record->name, nodename.c_str(), vname.c_str());
  }
  return 1;
}

bool NSM2CA::init_data_in(const char* recordname)
{
  StringList str = StringUtil::split(recordname, ':');
  if (str.size() >= 3 || str[0] == CAPITAL) {
    std::string name = StringUtil::toupper(str[1]);
    std::string format = str[2];
    try {
      NSM2CACallback::get().openData(name, format);
    } catch (const NSMHandlerException& e) {
      LogFile::error(e.what());
      return false;
    }
    return true;
  }
  return false;
}

long NSM2CA::read_data_in(aiRecord* record)
{
  StringList str = StringUtil::split(record->name, ':');
  if (str.size() >= 3 || str[0] == CAPITAL) {
    std::string name = StringUtil::toupper(str[1]);
    NSMData& data(NSM2CACallback::get().getData(name));
    if (!data.isAvailable()) return 1;
    DBField::Type type;
    int length;
    const void* buf = data.find(StringUtil::join(str, ".", 3), type, length);
    if (type == DBField::FLOAT) {
      record->val = *(const float*)buf;
    } else if (type == DBField::DOUBLE) {
      record->val = *(const double*)buf;
    }
    record->udf = FALSE;
    return 2;
  }
  return 0;
}

long NSM2CA::read_data_in(longinRecord* record)
{
  StringList str = StringUtil::split(record->name, ':');
  if (str.size() >= 3 || str[0] == CAPITAL) {
    std::string name = StringUtil::toupper(str[1]);
    NSMData& data(NSM2CACallback::get().getData(name));
    if (!data.isAvailable()) {
      return 1;
      }
    DBField::Type type;
    int length;
    const void* buf = data.find(StringUtil::join(str, ".", 3), type, length);
    if (type == DBField::CHAR) {
      record->val = *(const char*)buf;
    } else if (type == DBField::CHAR) {
      record->val = *(const byte8*)buf;
    } else if (type == DBField::SHORT) {
      record->val = *(const int16*)buf;
    } else if (type == DBField::SHORT) {
      record->val = *(const uint16*)buf;
    } else if (type == DBField::INT) {
      record->val = *(const int32*)buf;
    } else if (type == DBField::INT) {
      record->val = *(const uint32*)buf;
    } else if (type == DBField::LONG) {
      record->val = *(const int64*)buf;
    } else if (type == DBField::LONG) {
      record->val = *(const uint64*)buf;
    }
    record->udf = FALSE;
    return 2;
  }
  LogFile::error(StringUtil::join(str, ".", 4));
  return 0;
}

bool NSM2CA::find(const std::string& name, const std::string& sufix,
		  std::string& node, std::string& vname)
{
  StringList str = StringUtil::split(name, ':');
  if (str.size() > 3 && str[0] == CAPITAL && str[1] == sufix) {
    node = str[2];
    vname = StringUtil::join(str, ".", 3);
    return true;
  }
  return false;
}

NSMMessage NSM2CA::wait(int timeout, const NSMMessage& msg_in, const std::string& name)
{
  lock();
  if (m_msg.find(name) == m_msg.end()) {
    m_msg.insert(std::map<std::string, NSMMessage>::value_type(name, msg_in));
  }
  NSMCommunicator::send(msg_in);
  if (!m_cond.wait(m_mutex, timeout)) {
    unlock();
    throw(TimeoutException("Timeout response from %s", name.c_str()));
  }
  NSMMessage msg = m_msg[name];
  unlock();
  return msg;
}

NSMVar NSM2CA::get(const std::string& name)
{
  lock();
  if (m_var.find(name) == m_var.end()) {
    unlock();
    throw(TimeoutException("Range error %s", name.c_str()));
  }
  NSMVar var = m_var[name].var;
  unlock();
  return var;
}

void NSM2CA::notify(const std::string& name, const NSMMessage& msg)
{
  lock();
  m_msg[name] = msg;
  m_cond.signal();
  unlock();
}

void NSM2CA::notify(const std::string& name, const NSMVar& var)
{
  lock();
  m_var[name].var = var;
  m_cond.signal();
  scanIoRequest(*(m_var[name].pvt));
  unlock();
}
