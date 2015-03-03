#ifndef _Belle2_TTDACallback_hh
#define _Belle2_TTDACallback_hh

#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/runcontrol/RCCallback.h"
#include "daq/slc/runcontrol/RCNode.h"

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/PThread.h>

namespace Belle2 {

  class TTDACallback : public RCCallback {

  public:
    TTDACallback(const RCNode& ttd)
      : RCCallback(5), m_ttdnode(ttd) {}
    virtual ~TTDACallback() throw() {}

  public:
    virtual bool initialize(const DBObject& obj) throw();
    virtual bool configure(const DBObject& obj) throw();
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual void pause() throw(RCHandlerException);
    virtual void resume() throw(RCHandlerException);
    virtual void recover() throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void timeout(NSMCommunicator& com) throw();
    void trigft() throw(RCHandlerException);

  private:
    RCNode m_ttdnode;

  private:
    void send(const NSMMessage& msg) throw(RCHandlerException);

  };

}

#endif
