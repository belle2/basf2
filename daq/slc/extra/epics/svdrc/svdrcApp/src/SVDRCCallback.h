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
    RCState& getStateTarget() throw() { return m_state_target; }
    bool isAborting() const throw() { return m_isabort; }
    void setAborting(bool isabort) throw() { m_isabort = isabort; }
    bool isLoading() const throw() { return m_isload; }
    void setLoading(bool isload) throw() { m_isload = isload; }
    void checkRCState();
    void setState(const RCState& rcstate)
    {
      RCCallback::setState(rcstate);
      if (!rcstate.isTransition()) {
        m_rcstate = rcstate;
      }
    }
  private:
    chid m_RCRqs;
    chid m_PSRqs;
    bool m_isabort;
    bool m_isload;
    RCState m_state_target;
    RCState m_rcstate;
    HVState m_pscstate;

  };

}

#endif
