#ifndef _Belle2_ArichHVMonitor_h
#define _Belle2_ArichHVMonitor_h

#include <daq/slc/hvcontrol/arich/ArichHVCommunicator.h>

namespace Belle2 {

  class ArichHVMonitor {

  public:
    ArichHVMonitor(ArichHVCommunicator* comm) : m_comm(comm) {}
    ~ArichHVMonitor() throw() {}

  public:
    void run();

  private:
    ArichHVCommunicator* m_comm;

  };

}

#endif
