#ifndef _Belle2_ExpRecoCallback_hh
#define _Belle2_ExpRecoCallback_hh

#include <daq/slc/nsm/RCCallback.h>

#include <daq/rfarm/manager/RFConf.h>
#include <daq/rfarm/manager/RFProcessManager.h>
#include <daq/rfarm/manager/RFSharedMem.h>

#include <framework/pcore/RingBuffer.h>

namespace Belle2 {

  class ExpRecoCallback : public RCCallback {

  public:
    ExpRecoCallback(NSMNode* node = NULL);
    virtual ~ExpRecoCallback() throw();

  public:
    virtual bool boot() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool recover() throw();
    virtual bool trigft() throw();

  private:
    RFConf* m_conf;
    RFProcessManager* m_proc;
    char* m_nodename;
    RFSharedMem* m_shm;
    RingBuffer* m_rbuf;
    int m_pid_basf2;
    int m_pid_receiver;

  };

}

#endif
