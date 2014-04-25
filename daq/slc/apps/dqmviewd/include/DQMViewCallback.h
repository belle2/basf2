#ifndef _Belle2_DQMViewCallback_h
#define _Belle2_DQMViewCallback_h

#include "daq/slc/apps/dqmviewd/DQMViewMaster.h"
#include "daq/slc/apps/dqmviewd/DQMFileReader.h"

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/dqm/DQMPackage.h>
#include <daq/slc/dqm/DQMHistMap.h>

#include <vector>

namespace Belle2 {

  class DQMViewCallback : public RCCallback {

  public:
    DQMViewCallback(const NSMNode& node, DQMViewMaster& master)
      : RCCallback(node), m_master(master) {}
    virtual ~DQMViewCallback() throw() {}

  public:
    virtual void init() throw();
    virtual bool boot() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool pause() throw();
    virtual bool recover() throw();
    virtual bool abort() throw();
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }

  private:
    DQMViewMaster& m_master;
    unsigned int m_expno;
    unsigned int m_runno;
    unsigned int m_count;
    Mutex m_mutex;

  };

}

#endif
