#ifndef _Belle2_DataSender
#define _Belle2_DataSender

#include <daq/slc/system/TCPSocket.h>

namespace Belle2 {

  class EnvMonitorMaster;

  class DataSender {

  public:
    DataSender(TCPSocket& socket, EnvMonitorMaster& master)
      : m_master(master), m_socket(socket) {}
    ~DataSender();

  public:
    void run();

  private:
    EnvMonitorMaster& m_master;
    TCPSocket m_socket;

  };

}

#endif
