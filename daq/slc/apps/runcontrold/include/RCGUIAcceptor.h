#ifndef _Belle2_RCGUIAcceptor_hh
#define _Belle2_RCGUIAcceptor_hh

#include <string>

namespace Belle2 {

  class RCMasterCallback;

  class RCGUIAcceptor {

  public:
    RCGUIAcceptor(const std::string& ip, int port,
                  RCMasterCallback* callback)
      : _ip(ip), _port(port), _callback(callback) {}
    ~RCGUIAcceptor() {}

  public:
    void run();

  private:
    std::string _ip;
    int _port;
    RCMasterCallback* _callback;

  };

}

#endif

