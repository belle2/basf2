#ifndef _Belle2_PXDHVControlCallback_h
#define _Belle2_PXDHVControlCallback_h

#include <daq/slc/hvcontrol/HVControlCallback.h>
#include <daq/slc/runcontrol/RCNode.h>

struct MYNODE {
  char value[20];
  chid mychid;
  evid myevid;
};

namespace Belle2 {

  class PXDHVControlCallback : public HVControlCallback {

    class PXDPSVHandler : public NSMVHandler {

    public:
      PXDPSVHandler(const std::string& name, const NSMVar& var);
      virtual ~PXDPSVHandler() throw() {}

    public:
      virtual bool handleGet(NSMVar& var);
      virtual bool handleSet(const NSMVar& var);

    private:
      chid m_cid;

    };

  public:
    PXDHVControlCallback(const NSMNode& node, const std::string& rcname);
    virtual ~PXDHVControlCallback() throw() {}

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
    const HVState& getStateTarget() const throw() { return m_state_target; }
    const RCState getRCState() const throw() { return m_rcnode.getState(); }
    void setRCState(const RCState& state) throw() { m_rcnode.setState(state); }
    void sendToRC(const RCCommand& cmd);
    void setStoppedByTrip(bool stopped) { m_stopped_by_trip = stopped; }
    bool isStoppedByTrip() const { return m_stopped_by_trip; }

  private:
    RCNode m_rcnode;
    bool m_stopped_by_trip;
    chid m_RCRqs;
    chid m_PSRqs;
    HVState m_state_target;

  };

}

#endif
