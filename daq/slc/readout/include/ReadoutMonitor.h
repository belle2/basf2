#ifndef _Belle2_ReadoutMonitor_h
#define _Belle2_ReadoutMonitor_h

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/readout/ronode_info.h>
#include <daq/slc/readout/ronode_status.h>

namespace Belle2 {

  class ReadoutMonitor {

  public:
    ReadoutMonitor(RCCallback* callback,
                   ronode_info* info,
                   ronode_status* status)
      : m_callback(callback), m_info(info),
        m_status(status) {}
    ~ReadoutMonitor() {}

  public:
    void run();
    static int checkConnection(const std::string& proc_name, int port);

  private:
    RCCallback* m_callback;
    ronode_info* m_info;
    ronode_status* m_status;

  };

}

#endif


