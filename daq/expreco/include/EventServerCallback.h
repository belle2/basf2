#ifndef _Belle2_EventServerCallback_hh
#define _Belle2_EventServerCallback_hh

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

  class EventServerCallback : public RCCallback {

  public:
    EventServerCallback();
    virtual ~EventServerCallback() noexcept;

  public:
    virtual void load(const DBObject&);
    virtual void start();
    virtual void stop();
    virtual void recover(const DBObject&);
    virtual void abort();

    void EventServerLogger();

  private:
    RFConf* m_conf;
    RFProcessManager* m_proc;
    RFLogManager* m_log;
    char* m_nodename;
    RFSharedMem* m_shm;
    int m_pid_server;
    pthread_t m_logthread;

  };

}

#endif
