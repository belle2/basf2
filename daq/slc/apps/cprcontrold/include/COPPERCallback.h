#ifndef _Belle2_COPPERCallback_h
#define _Belle2_COPPERCallback_h

#include "daq/slc/apps/cprcontrold/FEEController.h"
#include "daq/slc/apps/cprcontrold/HSLBController.h"
#include "daq/slc/apps/cprcontrold/TTRXController.h"
#include "daq/slc/apps/cprcontrold/COPPERController.h"
#include "daq/slc/apps/cprcontrold/COPPERConfig.h"

#include <daq/slc/readout/ProcessController.h>
#include <daq/slc/readout/FlowMonitor.h>

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/LogFile.h>

namespace Belle2 {

  class COPPERCallback : public RCCallback {

  public:
    COPPERCallback(FEEController* fee[4]);
    virtual ~COPPERCallback() throw();

  public:
    virtual bool initialize(const DBObject& obj) throw();
    virtual bool configure(const DBObject& obj) throw();
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual void recover() throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void term() throw();
    virtual void timeout(NSMCommunicator& com) throw();
  public:
    HSLBController& getHSLB(int i) { return m_hslb[i]; }
    FEEController* getFEE(int i) { return m_fee[i]; }
    TTRXController& getTTRX() { return m_ttrx; }
    COPPERController& getCopper() { return m_copper; }

  private:
    ProcessController m_con;
    HSLBController m_hslb[4];
    FEEController* m_fee[4];
    TTRXController m_ttrx;
    COPPERController m_copper;
    FlowMonitor m_flow;
    COPPERConfig m_config;
    NSMData m_data;
    bool m_dummymode;
    bool m_force_boothslb;
    std::string m_hslb_firm;
    bool m_iserr;

  private:
    void bootBasf2() throw(RCHandlerException);
    void logging(bool err, LogFile::Priority pri,
                 const char* str, ...) throw();
    bool isError() throw();

  };

}

#endif
