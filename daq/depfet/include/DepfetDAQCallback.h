#ifndef _Belle2_DepfetDAQCallback_hh
#define _Belle2_DepfetDAQCallback_hh

#include <daq/storage/SharedEventBuffer.h>

#include <sys/types.h>
#include <netinet/in.h>

#include <daq/slc/apps/storagerd/mmap_statistics.h>

#include <daq/slc/readout/ProcessController.h>
#include <daq/slc/readout/FlowMonitor.h>

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/base/ConfigFile.h>

#include <vector>

namespace Belle2 {

  class DepfetDAQCallback : public RCCallback {

  public:
    DepfetDAQCallback();
    virtual ~DepfetDAQCallback() throw();

  public:
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual void configure(const DBObject& obj) throw(RCHandlerException);
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void monitor() throw(RCHandlerException);
    virtual void term() throw();

  private:
    ProcessController m_eb2rx;
    std::vector<ProcessController> m_con;
    std::vector<FlowMonitor> m_flow;
    NSMData m_data;
    std::vector<SharedEventBuffer> m_buf;
    ConfigFile m_file;
    Time m_time;
    int m_errcount;
    double m_t0;

  };

}

#endif
