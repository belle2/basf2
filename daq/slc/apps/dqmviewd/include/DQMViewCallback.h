#ifndef _Belle2_DQMViewCallback_h
#define _Belle2_DQMViewCallback_h

#include "daq/slc/apps/dqmviewd/DQMViewMaster.h"
#include "daq/slc/apps/dqmviewd/DQMFileReader.h"

#include <daq/slc/nsm/RCCallback.h>

#include <daq/slc/dqm/DQMPackage.h>
#include <daq/slc/dqm/DQMHistMap.h>

#include <vector>

namespace Belle2 {

  class DQMViewCallback : public RCCallback {

  public:
    DQMViewCallback(NSMNode* node, DQMViewMaster* master)
      : RCCallback(node), _master(master) {}
    virtual ~DQMViewCallback() throw() {}

  public:
    virtual void init() throw();
    virtual bool boot() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool abort() throw();
    void lock() { _mutex.lock(); }
    void unlock() { _mutex.unlock(); }

  private:
    DQMViewMaster* _master;
    unsigned int _expno;
    unsigned int _runno;
    unsigned int _count;
    Mutex _mutex;
    Cond _cond;

  };

}

#endif
