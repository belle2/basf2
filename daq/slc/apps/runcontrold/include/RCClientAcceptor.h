#ifndef _Belle2_RCClientAcceptor_hh
#define _Belle2_RCClientAcceptor_hh

#include <string>

namespace Belle2 {

  class RCMaster;

  class RCClientAcceptor {

  public:
    RCClientAcceptor(const std::string& ip, int port,
                     RCMaster* master)
      : _ip(ip), _port(port), _master(master) {}
    ~RCClientAcceptor() {}

  public:
    void run();

  private:
    std::string _ip;
    int _port;
    RCMaster* _master;

  };

}

#endif

