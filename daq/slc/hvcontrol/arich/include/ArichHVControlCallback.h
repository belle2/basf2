#ifndef _Belle2_ArichHVControlCallback_h
#define _Belle2_ArichHVControlCallback_h

#include <daq/slc/hvcontrol/HVControlCallback.h>

#include <vector>

namespace Belle2 {

  class ArichHVCommunicator;

  class ArichHVControlCallback : public HVControlCallback {

  public:
    ArichHVControlCallback(const NSMNode& node) throw()
      : HVControlCallback(node) {}
    virtual ~ArichHVControlCallback() throw();

  public:
    virtual void initialize() throw();
    virtual bool configure() throw();
    virtual bool turnon() throw();
    virtual bool turnoff() throw();
    virtual bool standby() throw();
    virtual bool shoulder() throw();
    virtual bool peak() throw();

  private:
    std::vector<ArichHVCommunicator*> m_comm_v;

  };

};

#endif
