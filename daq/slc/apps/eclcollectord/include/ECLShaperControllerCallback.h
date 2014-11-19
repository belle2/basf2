#ifndef _Belle2_ECLShaperControllerCallback_h
#define _Belle2_ECLShaperControllerCallback_h

#include "daq/slc/apps/eclcollectord/ECLShaperController.h"
#include "daq/slc/apps/eclcollectord/ECLShaperConfig.h"

#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/runcontrol/RCCallback.h"

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/PThread.h>

namespace Belle2 {

  class ECLShaperControllerCallback : public RCCallback {

  public:
    ECLShaperControllerCallback(const NSMNode& node);
    virtual ~ECLShaperControllerCallback() throw();

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
    ECLShaperController m_con;
    ECLShaperConfig m_config;
    NSMData m_data;
    bool m_forced;

  };

}

#endif
