#ifndef _Belle2_SocketAcceptor_hh
#define _Belle2_SocketAcceptor_hh

#include <string>

namespace Belle2 {

  class HistoServer;

  class SocketAcceptor {

  public:
    SocketAcceptor(const std::string& ip, HistoServer* server)
      : _ip(ip), _server(server) {}
    ~SocketAcceptor() {}

  public:
    void run();

  private:
    std::string _ip;
    HistoServer* _server;

  };

}

#endif

