#ifndef _Belle2_SocketAcceptor_hh
#define _Belle2_SocketAcceptor_hh

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/PThread.h>

#include <string>

namespace Belle2 {

  template<class WORKER, class ARGUMENT>
  class SocketAcceptor {

  public:
    SocketAcceptor(const std::string& ip,
                   int port, ARGUMENT* arg)
      : _ip(ip), _port(port), _arg(arg) {}
    ~SocketAcceptor() {}

  public:
    void run() {
      TCPServerSocket server_socket(_ip, _port);
      server_socket.open();
      while (true) {
        TCPSocket socket = server_socket.accept();
        PThread(new WORKER(socket, _arg));
      }
    }

  private:
    std::string _ip;
    int _port;
    ARGUMENT* _arg;

  };

}

#endif

