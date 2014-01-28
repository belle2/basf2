#ifndef _Belle2_SocketAcceptor_hh
#define _Belle2_SocketAcceptor_hh

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/PThread.h>
#include <daq/slc/system/LogFile.h>

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
        LogFile::debug("new connection accepted (%s:%d)", _ip.c_str(), _port);
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

