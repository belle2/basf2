#ifndef _Belle2_SVDHVControlCallback_h
#define _Belle2_SVDHVControlCallback_h

#include <daq/slc/hvcontrol/HVControlCallback.h>

struct MYNODE {
  char value[20];
  chid mychid;
  evid myevid;
};

namespace Belle2 {

  class SVDHVControlCallback : public HVControlCallback {

    class SVDPSVHandler : public NSMVHandler {

    public:
      SVDPSVHandler(const std::string& name, const NSMVar& var);
      virtual ~SVDPSVHandler() throw() {}

    public:
      virtual bool handleGet(NSMVar& var);
      virtual bool handleSet(const NSMVar& var);

    private:
      chid m_cid;

    };

  public:
    SVDHVControlCallback(const NSMNode& node);
    virtual ~SVDHVControlCallback() throw() {}

  public:
    virtual void turnon() throw(HVHandlerException);
    virtual void turnoff() throw(HVHandlerException);
    virtual void standby() throw(HVHandlerException);
    virtual void peak() throw(HVHandlerException);
    virtual void recover() throw(HVHandlerException);
    virtual void configure(const HVConfig&) throw(HVHandlerException) {}
    virtual void initialize(const HVConfig&) throw() {}
    virtual void init(NSMCommunicator&) throw();

  public:
    int putPV(chid cid, const char* val);
    bool addPV(const std::string& pvname) throw();
    HVState& getStateTarget() throw() { return m_state_target; }

  private:
    chid m_RCRqs;
    chid m_PSRqs;
    HVState m_state_target;

  };

}

#endif
