#ifndef _Belle2_RoiSenderCallback_hh
#define _Belle2_RoiSenderCallback_hh

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

  class RoiSenderCallback : public RCCallback {

  public:
    RoiSenderCallback();
    virtual ~RoiSenderCallback() throw();

  public:
    virtual void load(const DBObject&) throw(RCHandlerException);
    virtual void start() throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual void recover(const DBObject&) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);

    void RoiSenderLogger();

  private:
    RFConf* m_conf;
    RFProcessManager* m_proc;
    RFLogManager* m_log;
    char* m_nodename;
    RFSharedMem* m_shm;
    int m_pid_merger;
    pthread_t m_logthread;

  };

}

#endif
