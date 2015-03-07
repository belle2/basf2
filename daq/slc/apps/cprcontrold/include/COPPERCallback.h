#ifndef _Belle2_COPPERCallback_h
#define _Belle2_COPPERCallback_h

#include "daq/slc/copper/FEE.h"
#include "daq/slc/copper/HSLB.h"
#include "daq/slc/copper/TTRX.h"
#include "daq/slc/copper/COPPER.h"

#include <daq/slc/readout/ProcessController.h>
#include <daq/slc/readout/FlowMonitor.h>

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/LogFile.h>

namespace Belle2 {

  class COPPERCallback : public RCCallback {

  public:
    COPPERCallback(FEE* fee[4]);
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
    HSLB& getHSLB(int i) { return m_hslb[i]; }
    FEE* getFEE(int i) { return m_fee[i]; }
    TTRX& getTTRX() { return m_ttrx; }
    COPPER& getCopper() { return m_copper; }

  private:
    ProcessController m_con;
    HSLB m_hslb[4];
    FEE* m_fee[4];
    TTRX m_ttrx;
    COPPER m_copper;
    FlowMonitor m_flow;
    NSMData m_data;
    bool m_dummymode;
    bool m_force_boothslb;
    std::string m_hslb_firm;
    bool m_iserr;

  private:
    void bootBasf2(const DBObject& obj) throw(RCHandlerException);
    void logging(bool err, LogFile::Priority pri,
                 const char* str, ...) throw();
    bool isError() throw();

  };

}

#endif
