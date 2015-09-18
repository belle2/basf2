#ifndef _Belle2_NSM2CA_h
#define _Belle2_NSM2CA_h

#include "NSM2CACallback.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/runcontrol/RCCommand.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <aiRecord.h>
#include <longinRecord.h>
#include <stringinRecord.h>
#include <aoRecord.h>
#include <longoutRecord.h>
#include <stringoutRecord.h>
#include <recGbl.h>
#include <dbAccess.h>
#include <dbScan.h>

#include <map>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>

#define CAPITAL "B2_nsm"

namespace Belle2 {

  class NSM2CA {

  public:
    template<typename T>
    long init_in(T* record, IOSCANPVT* pvt);
    template<typename T>
    long init_out(T* record);
    template<typename T>
    long get_ioint_info_in(int cmd, T* record, IOSCANPVT* pvt);
    template<typename T, typename V>
    long read_in(T* record, V val);
    long read_in(stringinRecord* record, const std::string& val);
    template<typename T>
    long init_data_in(T* record);
    long read_data_in(aiRecord* record);
    long read_data_in(longinRecord* record);
    template <typename T>
    long init_vget_in(T* record, const std::string& type);
    long read_vget_in(aiRecord* record);
    long read_vget_in(longinRecord* record);
    long read_vget_in(stringinRecord* record);
    template <typename T>
    long init_vset_out(T* record);
    long write_vset_out(aoRecord* record);
    long write_vset_out(longoutRecord* record);
    long write_vset_out(stringoutRecord* record);

  private:
    bool find(const std::string& name, const std::string& sufix,
              std::string& node, std::string& vname);

  private:
    std::map<std::string, IOSCANPVT*> m_pvt;

  };

  // implementaion of template functions

  template<typename T>
  inline long NSM2CA::init_in(T* record, IOSCANPVT* pvt)
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
    LogFile::debug("init_in: %s", record->name);
    scanIoInit(pvt);
    record->dpvt = pvt;
    return 0;
  }

  template<typename T>
  inline long NSM2CA::init_out(T* record)
  {
    if (record->scan != SCAN_PASSIVE) {
      recGblRecordError(S_db_badField, (void*)record,
                        "init_record Illegal SCAN field. PASSIVE scans only.");
      return S_db_badField;
    }
    return 0;
  }

  template<typename T>
  inline long NSM2CA::get_ioint_info_in(int cmd, T* record, IOSCANPVT* pvt)
  {
    if (cmd != 0) return 0;
    if (!record->dpvt) return 1;
    *pvt = *((IOSCANPVT*)record->dpvt);
    return 0;
  }

  template<typename T, typename V>
  inline long NSM2CA::read_in(T* record, V val)
  {
    record->val = val;
    record->udf = FALSE;
    return 0;
  }

  inline long NSM2CA::read_in(stringinRecord* record, const std::string& val)
  {
    memset(record->val, 0, strlen(record->val));
    strcpy(record->val, val.c_str());
    record->udf = FALSE;
    return 0;
  }

  template<typename T>
  inline long NSM2CA::init_data_in(T* record)
  {
    StringList str = StringUtil::split(record->name, ':');
    LogFile::error("%s %d", record->name, (int)str.size());
    if (str.size() >= 3 || str[0] == CAPITAL) {
      IOSCANPVT* pvt = new IOSCANPVT;
      std::string name = StringUtil::toupper(str[1]);
      std::string format = str[2];
      NSM2CACallback::get().openData(name, format);
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

  inline long NSM2CA::read_data_in(aiRecord* record)
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
        return read_in(record, *(const float*)buf);
      } else if (type == DBField::DOUBLE) {
        return read_in(record, *(const double*)buf);
      }
    }
    return 0;
  }

  inline long NSM2CA::read_data_in(longinRecord* record)
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
        return read_in(record, *(const char*)buf);
      } else if (type == DBField::CHAR) {
        return read_in(record, *(const byte8*)buf);
      } else if (type == DBField::SHORT) {
        return read_in(record, *(const int16*)buf);
      } else if (type == DBField::SHORT) {
        return read_in(record, *(const uint16*)buf);
      } else if (type == DBField::INT) {
        return read_in(record, *(const int32*)buf);
      } else if (type == DBField::INT) {
        return read_in(record, *(const uint32*)buf);
      } else if (type == DBField::LONG) {
        return read_in(record, *(const int64*)buf);
      } else if (type == DBField::LONG) {
        return read_in(record, *(const uint64*)buf);
      }
    }
    LogFile::error(StringUtil::join(str, ".", 4));
    return 0;
  }

  inline bool NSM2CA::find(const std::string& name, const std::string& sufix,
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

  template <typename T>
  inline long NSM2CA::init_vget_in(T* record, const std::string& type)
  {
    std::string node, vname;
    if (find(record->name, "get", node, vname)) {
      IOSCANPVT* pvt = new IOSCANPVT;
      try {
        NSMVHandler2CA* handler = new NSMVHandler2CA(node, vname, *pvt, type);
        NSM2CACallback::get().add(handler);
        NSMCommunicator::send(NSMMessage(NSMNode(node), NSMCommand::VGET, vname));
      } catch (const NSMHandlerException& e) {
        LogFile::error(e.what());
      } catch (const TimeoutException& e) {
      }
      return init_in(record, pvt);
    }
    return 1;
  }

  inline long NSM2CA::read_vget_in(aiRecord* record)
  {
    std::string node, vname;
    if (find(record->name, "get", node, vname)) {
      float val = 0;
      NSM2CACallback::get().get(node, vname, val);
      return read_in(record, val);
    }
    return 0;
  }

  inline long NSM2CA::read_vget_in(longinRecord* record)
  {
    std::string node, vname;
    if (find(record->name, "get", node, vname)) {
      int val = 0;
      NSM2CACallback::get().get(node, vname, val);
      return read_in(record, val);
    }
    return 0;
  }

  inline long NSM2CA::read_vget_in(stringinRecord* record)
  {
    std::string node, vname;
    if (find(record->name, "get", node, vname)) {
      std::string val;
      NSM2CACallback::get().get(node, vname, val);
      return read_in(record, val);
    }
    return 0;
  }

  template <typename T>
  inline long NSM2CA::init_vset_out(T* record)
  {
    std::string nodename, vname;
    if (find(record->name, "set", nodename, vname)) {
      return init_out(record);
    }
    return 0;
  }

  inline long NSM2CA::write_vset_out(aoRecord* record)
  {
    LogFile::debug("write a %s << %f", record->name, record->val);
    std::string nodename, vname;
    if (find(record->name, "set", nodename, vname)) {
      NSMNode node(nodename);
      NSM2CACallback::get().set(node, vname, (float)record->val);
      return 0;
    }
    return 1;
  }

  inline long NSM2CA::write_vset_out(longoutRecord* record)
  {
    LogFile::debug("write long %s << %d", record->name, record->val);
    std::string nodename, vname;
    if (find(record->name, "set", nodename, vname)) {
      NSMNode node(nodename);
      try {
        NSM2CACallback::get().set(node, vname, record->val);
      } catch (const TimeoutException& e) {
      }
      return 0;
    }
    return 1;
  }

  inline long NSM2CA::write_vset_out(stringoutRecord* record)
  {
    LogFile::debug("write string %s << %s", record->name, record->val);
    std::string nodename, vname;
    if (find(record->name, "set", nodename, vname)) {
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
      } else if (vname == "rcstate") {
        NSM2CACallback::get().set(vname, record->val);
      } else {
        LogFile::error("%s set %s %s %s", record->name, nodename.c_str(), vname.c_str(), record->val);
        try {
          NSM2CACallback::get().set(NSMNode(node), vname, record->val);
        } catch (const IOException& e) {
          LogFile::error(e.what());
        }
      }
      return 0;
    } else {
      LogFile::error("error : %s set %s %s", record->name, nodename.c_str(), vname.c_str());
    }
    return 0;
  }

}

#endif
