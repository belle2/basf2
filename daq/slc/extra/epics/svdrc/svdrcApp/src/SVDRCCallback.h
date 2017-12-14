#ifndef _Belle2_SVDRCCallback_h
#define _Belle2_SVDRCCallback_h

#include <daq/slc/runcontrol/RCCallback.h>
#include <daq/slc/hvcontrol/HVState.h>

struct MYNODE {
  char value[20];
  chid mychid;
  evid myevid;
};

namespace Belle2 {

  class SVDRCCallback : public RCCallback {

    class SVDRCVHandler : public NSMVHandler {

    public:
      SVDRCVHandler(const std::string& name, const NSMVar& var);
      virtual ~SVDRCVHandler() throw() {}

    public:
      virtual bool handleGet(NSMVar& var);
      virtual bool handleSet(const NSMVar& var);

    private:
      chid m_cid;

    };

  public:
    SVDRCCallback(const NSMNode& node);
    virtual ~SVDRCCallback() throw() {}

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
    int putPV(chid cid, int val);
    bool addPV(const std::string& pvname) throw();
    void setStateRequest(const RCState& state) throw() { m_state_req = state; }

  private:
    chid m_RC_req;
    RCState m_state_req;
    HVState m_pscstate;

  };

}

#endif
