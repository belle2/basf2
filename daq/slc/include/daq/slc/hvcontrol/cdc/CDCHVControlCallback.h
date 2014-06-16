#ifndef _Belle2_CDCHVControlCallback_h
#define _Belle2_CDCHVControlCallback_h

#include <daq/slc/hvcontrol/HVControlCallback.h>

#include <vector>

namespace Belle2 {

  class CDCHVCommunicator;

  class CDCHVControlCallback : public HVControlCallback {

  public:
    CDCHVControlCallback(const NSMNode& node) throw()
      : HVControlCallback(node) {}
    virtual ~CDCHVControlCallback() throw();

  public:
    virtual void initialize() throw();
    virtual bool configure() throw();
    virtual bool turnon() throw();
    virtual bool turnoff() throw();
    virtual bool standby() throw();
    virtual bool shoulder() throw();
    virtual bool peak() throw();
    virtual bool hvapply(const HVApplyMessage& hvmsg) throw();

  private:
    std::vector<CDCHVCommunicator*> m_comm_v;

  };

};

#endif
