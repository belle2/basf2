#ifndef _Belle2_ArichHVControlCallback_h
#define _Belle2_ArichHVControlCallback_h

#include <daq/slc/apps/hvcontrold/HVControlCallback.h>

#include <vector>

namespace Belle2 {

  class ArichHVCommunicator;

  class ArichHVControlCallback : public HVControlCallback {

  public:
    ArichHVControlCallback(NSMNode* node) throw()
      : HVControlCallback(node) {}
    virtual ~ArichHVControlCallback() throw();

  public:
    virtual void initialize() throw();
    virtual bool configure() throw();
    virtual bool turnon() throw();
    virtual bool turnoff() throw();
    virtual bool standby() throw();
    virtual bool standby2() throw();
    virtual bool standby3() throw();
    virtual bool peak() throw();

  private:
    bool waitStable() throw();

  private:
    std::vector<ArichHVCommunicator*> m_comm_v;

  };

};

#endif
