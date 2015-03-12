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

  class RFRunControlCallback;

  class RFMasterCallback : public RCCallback {

  public:
    RFMasterCallback() : m_callback(NULL) {}
    virtual ~RFMasterCallback() throw() {}

  public:
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual void configure(const DBObject& obj) throw(RCHandlerException);
    virtual void timeout(NSMCommunicator& com) throw();
    virtual void ok(const char* node, const char* data) throw();
    virtual void error(const char* node, const char* data) throw();
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);
    virtual void resume(int subno) throw(RCHandlerException);
    virtual void pause() throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);

  public:
    void setState(NSMNode& node, const RCState& state);
    void setCallback(RFRunControlCallback* callback) { m_callback = callback; }

  private:
    RFRunControlCallback* m_callback;
    NSMDataList m_datas;
    typedef std::vector<NSMNode> NSMNodeList;
    NSMNodeList m_nodes;

  };

}

#endif
