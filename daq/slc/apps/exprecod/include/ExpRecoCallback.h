#ifndef _Belle2_ExpRecoCallback_hh
#define _Belle2_ExpRecoCallback_hh

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/rfarm/manager/RFConf.h>
#include <daq/rfarm/manager/RFProcessManager.h>
#include <daq/rfarm/manager/RFSharedMem.h>
#include <daq/rfarm/manager/RFLogManager.h>

#include <framework/pcore/RingBuffer.h>

namespace Belle2 {

  class ExpRecoCallback : public RCCallback {

  public:
    ExpRecoCallback();
    virtual ~ExpRecoCallback() throw();

  public:
    virtual void load(const DBObject&) throw(RCHandlerException);
    virtual void start() throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual void recover(const DBObject&) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);

    void ExpRecoLogger();

  private:
    RFConf* m_conf;
    RFProcessManager* m_proc;
    RFLogManager* m_log;
    char* m_nodename;
    RFSharedMem* m_shm;
    RingBuffer* m_rbufin;
    RingBuffer* m_rbufout;
    int m_pid_basf2;
    int m_pid_receiver;
    int m_pid_evs;
    pthread_t m_logthread;

  };

}

#endif
