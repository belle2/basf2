#ifndef _Belle2_RCMasterCallback_h
#define _Belle2_RCMasterCallback_h

#include <daq/slc/nsm/RCCallback.h>

#include <daq/slc/system/Mutex.h>

#include <daq/slc/base/Command.h>

#include <list>

namespace Belle2 {

  class RCCommunicator;
  class RCMaster;

  class RCMasterCallback : public RCCallback {

  public:
    RCMasterCallback(NSMNode* node);
    virtual ~RCMasterCallback() throw();
    void setMaster(RCMaster* master) { _master = master; }
    RCMaster* getMaster() { return _master; }

  private:
    bool distribute(Command command, int num0, int num1, int num2);

  public:
    virtual void init() throw();
    virtual bool boot() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool trigft() throw();
    virtual bool recover() throw();
    virtual bool abort() throw();
    virtual bool pause() throw();
    virtual bool resume() throw();

  private:
    RCMaster* _master;

  };

}

#endif
