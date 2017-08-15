#ifndef _Belle2_RFMasterCallback_h
#define _Belle2_RFMasterCallback_h

#include "daq/rfarm/manager/RFConf.h"

#include <daq/slc/runcontrol/RCState.h>
#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/base/StringUtil.h>

#include <vector>
#include <map>

namespace Belle2 {

  class ERecoRunControlCallback;

  class ERecoMasterCallback : public RCCallback {

  public:
    ERecoMasterCallback() throw();
    virtual ~ERecoMasterCallback() throw() {}

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
    void setCallback(ERecoRunControlCallback* callback) { m_callback = callback; }
    void addData(const std::string& dataname, const std::string& format);

  private:
    ERecoRunControlCallback* m_callback;
    StringList m_dataname;
    typedef std::vector<NSMNode> NSMNodeList;
    NSMNodeList m_nodes;

  };

}

#endif
