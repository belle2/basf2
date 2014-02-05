#ifndef _Belle2_DataSender
#define _Belle2_DataSender

#include <daq/slc/system/TCPSocket.h>

namespace Belle2 {

  class EnvMonitorMaster;

  class DataSender {

  public:
    DataSender(TCPSocket& socket, EnvMonitorMaster* master)
      : _master(master), _socket(socket) {}
    ~DataSender();

  public:
    void run();

  private:
    EnvMonitorMaster* _master;
    TCPSocket _socket;

  };

}

#endif
