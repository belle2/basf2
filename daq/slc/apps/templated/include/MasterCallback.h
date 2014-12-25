#ifndef _Belle2_MasterCallback_hh
#define _Belle2_MasterCallback_hh

#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/runcontrol/RCCallback.h"

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/PThread.h>

namespace Belle2 {

  class MasterCallback : public RCCallback {

  public:
    MasterCallback(const NSMNode& node);
    virtual ~MasterCallback() throw();

  public:
    virtual void init() throw();
    virtual void term() throw();
    virtual void timeout() throw();
    virtual bool ok() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool resume() throw();
    virtual bool pause() throw();
    virtual bool recover() throw();
    virtual bool abort() throw();
    virtual bool trigft() throw();

  private:
    NSMData m_data;
    int m_count;

  };

}

#endif
