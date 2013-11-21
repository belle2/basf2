#ifndef _Belle2_SocketAcceptor_hh
#define _Belle2_SocketAcceptor_hh

#include "EnvMonitorMaster.h"

#include <string>

namespace Belle2 {

  class SocketAcceptor {

  public:
    SocketAcceptor(const std::string& ip, int port,
                   EnvMonitorMaster* master)
      : _ip(ip), _port(port), _master(master) {}
    ~SocketAcceptor() {}

  public:
    void run();

  private:
    std::string _ip;
    int _port;
    EnvMonitorMaster* _master;

  };

}

#endif

