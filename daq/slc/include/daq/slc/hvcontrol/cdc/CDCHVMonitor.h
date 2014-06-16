#ifndef _Belle2_CDCHVMonitor_h
#define _Belle2_CDCHVMonitor_h

#include <daq/slc/hvcontrol/cdc/CDCHVCommunicator.h>

namespace Belle2 {

  class CDCHVMonitor {

  public:
    CDCHVMonitor(CDCHVCommunicator* comm) : m_comm(comm) {}
    ~CDCHVMonitor() throw() {}

  public:
    void run();

  private:
    CDCHVCommunicator* m_comm;

  };

}

#endif
