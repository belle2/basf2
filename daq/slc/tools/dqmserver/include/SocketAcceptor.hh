#ifndef _B2DQM_SocketAcceptor_hh
#define _B2DQM_SocketAcceptor_hh

namespace B2DQM {

  class HistoServer;

  class SocketAcceptor {
    
  public:
    SocketAcceptor(HistoServer* server)
      : _server(server) {}
    ~SocketAcceptor() {}
    
  public:
    void run();

  private:
    HistoServer* _server;
    
  };

}

#endif

