#ifndef _Belle2_HVMasterCallback_hh
#define _Belle2_HVMasterCallback_hh

#include "daq/slc/apps/hvcontrold/HVCallback.h"

namespace Belle2 {

  class HVControlMaster;

  class HVMasterCallback : public HVCallback {

  public:
    HVMasterCallback(NSMNode* node, HVControlMaster* master) throw();
    virtual ~HVMasterCallback() throw() {}

  public:
    virtual void init() throw();
    virtual bool ok() throw();
    virtual bool error() throw();
    virtual bool turnon() throw();
    virtual bool turnoff() throw();
    virtual bool rampup() throw();
    virtual bool rampdown() throw();
    virtual bool standby() throw();
    virtual bool standby2() throw();
    virtual bool standby3() throw();
    virtual bool peak() throw();
    virtual bool config() throw();
    virtual bool save() throw();
    virtual void selfCheck() throw();

  protected:
    virtual bool perform(NSMMessage& msg) throw(NSMHandlerException);

  private:
    HVControlMaster* _master;
    NSMCommunicator* _comm;

  };

};

#endif
