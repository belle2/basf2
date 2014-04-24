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
                   int port, ARGUMENT& arg)
      : m_ip(ip), m_port(port), m_arg(arg) {}
    ~SocketAcceptor() {}

  public:
    void run() {
      TCPServerSocket server_socket(m_ip, m_port);
      server_socket.open();
      while (true) {
        TCPSocket socket = server_socket.accept();
        LogFile::debug("new connection accepted (%s:%d)",
                       m_ip.c_str(), m_port);
        PThread(new WORKER(socket, m_arg));
      }
    }

  private:
    std::string m_ip;
    int m_port;
    ARGUMENT& m_arg;

  };

}

#endif

