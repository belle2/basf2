#ifndef _B2DQM_SocketAcceptor_hh
#define _B2DQM_SocketAcceptor_hh

#include <string>

namespace B2DQM {

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

