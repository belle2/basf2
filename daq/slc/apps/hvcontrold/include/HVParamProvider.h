#ifndef _Belle2_HVParamProvider_h
#define _Belle2_HVParamProvider_h

#include <daq/slc/system/TCPSocket.h>

namespace Belle2 {

  class HVControlMaster;

  class HVParamProvider {

  public:
    HVParamProvider(TCPSocket& socket,
                    HVControlMaster* master)
      : _socket(socket), _master(master) {}
    ~HVParamProvider() {}

  public:
    void run();

  private:
    TCPSocket _socket;
    HVControlMaster* _master;

  };

}

#endif

