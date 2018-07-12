#ifndef _Belle2_EB2TXCallback_h
#define _Belle2_EB2TXCallback_h

#include "daq/slc/runcontrol/RCCallback.h"
#include "daq/slc/readout/ProcessController.h"
#include "daq/slc/apps/eb2txd/mmap_statistics.h"

namespace Belle2 {

  class EB2TXCallback : public RCCallback {

  public:
    EB2TXCallback();
    virtual ~EB2TXCallback() throw();

  public:
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual void configure(const DBObject& obj) throw(RCHandlerException);
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual bool pause() throw(RCHandlerException);
    virtual bool resume(int subno) throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void monitor() throw(RCHandlerException);

  private:
    ProcessController m_con;
    eb_statistics* m_eb_stat;
    int m_nsenders;
    double m_nevent_in[10];
    double m_total_byte_in[10];
    double m_nevent_out[10];
    double m_total_byte_out[10];
    double m_t0;

  };

}

#endif
