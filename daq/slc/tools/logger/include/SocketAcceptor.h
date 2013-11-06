#ifndef _Belle2_SocketAcceptor_hh
#define _Belle2_SocketAcceptor_hh

#include <string>

namespace Belle2 {

  class SocketAcceptor {

  public:
    SocketAcceptor(const std::string& ip, int port)
      : _ip(ip), _port(port) {}
    ~SocketAcceptor() {}

  public:
    void run();

  private:
    std::string _ip;
    int _port;

  };

}

#endif

