#ifndef _Belle2_RFMasterCallback_h
#define _Belle2_RFMasterCallback_h

#include "daq/rfarm/manager/RFConf.h"
#include "daq/rfarm/manager/RFCommand.h"

#include <daq/slc/runcontrol/RCState.h>
#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <vector>
#include <map>

namespace Belle2 {

  class RFRunControlCallback;

  class RFMasterCallback : public RCCallback {

  public:
    RFMasterCallback(ConfigFile& config) throw();
    virtual ~RFMasterCallback() throw() {}

  public:
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual void configure(const DBObject& obj) throw(RCHandlerException);
    virtual void monitor() throw(RCHandlerException);
    virtual void ok(const char* node, const char* data) throw();
    virtual void error(const char* node, const char* data) throw();
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);
    virtual bool resume(int subno) throw(RCHandlerException);
    virtual bool pause() throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual bool perform(NSMCommunicator& com) throw();

  public:
    void setState(NSMNode& node, const RCState& state);
    void setCallback(RFRunControlCallback* callback) { m_callback = callback; }
    void addData(const std::string& dataname, const std::string& format);

  private:
    RFRunControlCallback* m_callback;
    StringList m_dataname;
    typedef std::vector<NSMNode> NSMNodeList;
    NSMNodeList m_nodes;
    int m_pxd_used;
    NSMNode m_rcnode;
    std::string m_script;
    std::string m_script_noroi;
    std::string m_script_sendroi;

  };

}

#endif
