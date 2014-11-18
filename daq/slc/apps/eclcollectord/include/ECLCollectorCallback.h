#ifndef _Belle2_ECLCollectorCallback_hh
#define _Belle2_ECLCollectorCallback_hh

#include "daq/slc/apps/eclcollectord/ECLCollectorController.h"

#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/runcontrol/RCCallback.h"

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/PThread.h>

namespace Belle2 {

  class ECLCollectorCallback : public RCCallback {

  public:
    ECLCollectorCallback(const NSMNode& node);
    virtual ~ECLCollectorCallback() throw();

  public:
    virtual void init() throw();
    virtual void term() throw();
    virtual void timeout() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool resume() throw();
    virtual bool pause() throw();
    virtual bool recover() throw();
    virtual bool abort() throw();

  private:
    ECLCollectorController m_collector;
    NSMData m_data;

  };

}

#endif
