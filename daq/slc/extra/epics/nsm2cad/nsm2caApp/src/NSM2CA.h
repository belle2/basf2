#ifndef _Belle2_NSM2CA_h
#define _Belle2_NSM2CA_h

#include "NSM2CACallback.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/runcontrol/RCCommand.h>
#include <daq/slc/hvcontrol/HVCommand.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/Cond.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <aiRecord.h>
#include <longinRecord.h>
#include <stringinRecord.h>
#include <aoRecord.h>
#include <longoutRecord.h>
#include <stringoutRecord.h>
#include <recGbl.h>

#include <map>

#define CAPITAL "B2_nsm"

namespace Belle2 {

  struct var_pvt {
    NSMVar var;
    IOSCANPVT* pvt;
    bool updated;
  };
  typedef std::map<std::string, var_pvt> NSMVarPVTList;

  class NSM2CA {

  public:
    IOSCANPVT* init_vget_in(const char* recordname);
    long read_vget_in(aiRecord* record);
    long read_vget_in(longinRecord* record);
    long read_vget_in(stringinRecord* record);
    bool init_vset_out(const char* recordname, void* record);
    long write_vset_out(aoRecord* record);
    long write_vset_out(longoutRecord* record);
    long write_vset_out(stringoutRecord* record);
    bool init_data_in(const char* recordname);
    long read_data_in(aiRecord* record);
    long read_data_in(longinRecord* record);

  public:
    NSMMessage wait(int timeout, const NSMMessage& msg_in, const std::string& name);
    NSMVar get(const std::string& name);
    float getFloat(const std::string& name) { return get(name).getFloat(); }
    int getInt(const std::string& name) { return get(name).getInt(); }
    std::string getText(const std::string& name) { return get(name).getText(); }
    void notify(const std::string& name, const NSMMessage& msg);
    void notify(const std::string& name, const NSMVar& var);
    NSMVarPVTList& getVars() { return m_var; }
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }

  private:
    bool find(const std::string& name, const std::string& sufix,
              std::string& node, std::string& vname);

  private:
    std::map<std::string, void*> m_orecords;
    std::map<std::string, void*> m_irecords;
    Mutex m_mutex;
    Cond m_cond;
    std::map<std::string, NSMMessage> m_msg;
    NSMVarPVTList m_var;

  };

}

#endif
