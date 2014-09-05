#ifndef _Belle2_SocketAcceptor_hh
#define _Belle2_SocketAcceptor_hh

#include <string>

namespace Belle2 {

  class DQMViewCallback;

  class SocketAcceptor {

  public:
    SocketAcceptor(const std::string& ip, int port,
                   DQMViewCallback* callback)
      : m_ip(ip), m_port(port), m_callback(callback) {}
    ~SocketAcceptor() {}

  public:
    void run();

  private:
    std::string m_ip;
    int m_port;
    DQMViewCallback* m_callback;

  };

}

#endif

