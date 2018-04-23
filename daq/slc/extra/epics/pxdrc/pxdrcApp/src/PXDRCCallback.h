#pragma once

#include <daq/slc/runcontrol/RCCallback.h>

struct MYNODE {
  char value[20];
  chid mychid;
  evid myevid;
};

namespace Belle2 {

  class PXDRCCallback : public RCCallback {

    class PXDRCVHandler : public NSMVHandler {

    public:
      PXDRCVHandler(const std::string& name, const NSMVar& var);
      virtual ~PXDRCVHandler() throw() {}

    public:
      virtual bool handleGet(NSMVar& var);
      virtual bool handleSet(const NSMVar& var);

    private:
      chid m_cid;

    };

  public:
    PXDRCCallback(const NSMNode& node);
    virtual ~PXDRCCallback() throw() {}

  public:
    static const char* pvRCcur;
    static const char* pvRCreq;

  public:
    virtual void boot(const DBObject&) throw(RCHandlerException) {}
    virtual void load(const DBObject&) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual bool pause() throw(RCHandlerException) { return true; }
    virtual bool resume(int /*subno*/) throw(RCHandlerException) { return true; }
    virtual void init(NSMCommunicator&) throw();

  public:
    int putPV(chid cid, const char* val);
    bool addPV(const std::string& pvname, chid& pv) throw();
    void setStateRequest(const RCState& state) throw() { m_state_req = state; }
    RCState getRCCurrent();
    RCState getRCRequest();

  private:
    chid m_RC_req;
    chid m_RC_cur;
    RCState m_state_req;

  };

}
