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
#include <daq/slc/system/SharedMemory.h>

namespace Belle2 {

  class COPPERCallback : public RCCallback {

  public:
    COPPERCallback(FEE* fee[4], bool dummymode, bool disablefeconf);
    virtual ~COPPERCallback() throw();

  public:
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual void configure(const DBObject& obj) throw(RCHandlerException);
    virtual void boot(const DBObject& obj) throw(RCHandlerException);
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual bool pause() throw(RCHandlerException);
    virtual bool resume(int subno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void term() throw();
    virtual void monitor() throw(RCHandlerException);

  public:
    HSLB& getHSLB(int i) { return m_hslb[i]; }
    FEE* getFEE(int i) { return m_fee[i]; }
    TTRX& getTTRX() { return m_ttrx; }
    COPPER& getCopper() { return m_copper; }
    ProcessController& getProcess() { return m_con; }
    virtual bool feeload();
    void getfee(HSLB& hslb, int& hwtype, int& serial, int& fwtype, int& fwver)  throw(HSLBHandlerException);
    DBObject& getFEEDB(int hslb) { return m_o_fee[hslb]; }

  private:
    ProcessController m_con;
    HSLB m_hslb[4];
    FEE* m_fee[4];
    TTRX m_ttrx;
    COPPER m_copper;
    FlowMonitor m_flow;
    bool m_dummymode;
    bool m_force_boothslb;
    std::string m_hslb_firm;
    bool m_iserr;
    SharedMemory m_memory;
    bool m_disablefeconf;
    DBObject m_o_fee[4];

  private:
    void bootBasf2(const DBObject& obj) throw(RCHandlerException);
    void logging(bool err, LogFile::Priority pri,
                 const char* str, ...) throw();
    bool isError() throw();

  };

}

#endif
