#ifndef _Belle2_DatabrigdeCallback_h
#define _Belle2_DatabrigdeCallback_h

#include "daq/slc/runcontrol/RCCallback.h"
#include "daq/slc/readout/ProcessController.h"
#include "daq/slc/apps/storagerd/mmap_statistics.h"

namespace Belle2 {

  class DatabrigdeCallback : public RCCallback {

  public:
    DatabrigdeCallback();
    virtual ~DatabrigdeCallback() throw();

  public:
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual void configure(const DBObject& obj) throw(RCHandlerException);
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual bool pause() throw(RCHandlerException);
    virtual bool resume(int subno) throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void monitor() throw(RCHandlerException);

  private:
    ProcessController m_con;
    eb_statistics* m_eb_stat;

  };

}

#endif
