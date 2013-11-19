#ifndef _Belle2_SocketAcceptor_hh
#define _Belle2_SocketAcceptor_hh

#include <system/TCPServerSocket.h>
#include <system/PThread.h>

#include <string>

namespace Belle2 {

  template <class WORKER>
  class SocketAcceptor {

  public:
    SocketAcceptor(const std::string& ip, int port)
      : _ip(ip), _port(port) {}
    ~SocketAcceptor() {}

  public:
    void run() {
      TCPServerSocket server_socket(_ip, _port);
      server_socket.open();
      while (true) {
        TCPSocket socket = server_socket.accept();
        PThread(new WORKER(socket));
      }
    }

  private:
    std::string _ip;
    int _port;

  };

}

#endif
