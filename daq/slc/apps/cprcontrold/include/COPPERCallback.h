#ifndef _Belle2_COPPERCallback_h
#define _Belle2_COPPERCallback_h

#include "daq/slc/apps/cprcontrold/FEEController.h"
#include "daq/slc/apps/cprcontrold/HSLBController.h"
#include "daq/slc/apps/cprcontrold/TTRXController.h"
#include "daq/slc/apps/cprcontrold/COPPERController.h"
#include "daq/slc/apps/cprcontrold/COPPERConfig.h"

#include "daq/slc/readout/ProcessController.h"
#include "daq/slc/readout/FlowMonitor.h"

#include "daq/slc/runcontrol/RCCallback.h"

#include "daq/slc/nsm/NSMData.h"

namespace Belle2 {

  class COPPERCallback : public RCCallback {

  public:
    COPPERCallback(const NSMNode& node,
                   FEEController* fee[4]);
    virtual ~COPPERCallback() throw();

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
    bool bootBasf2() throw();

  private:
    ProcessController m_con;
    HSLBController m_hslb[4];
    FEEController* m_fee[4];
    TTRXController m_ttrx;
    COPPERController m_copper;
    FlowMonitor m_flow;
    COPPERConfig m_config;
    NSMData m_data;

  };

}

#endif
