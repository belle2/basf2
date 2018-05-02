#pragma once

#include <framework/pcore/zmq/processModules/ZMQTxModule.h>

namespace Belle2 {
  class ZMQTxWorkerModule : public ZMQTxModule {
  public:
    ZMQTxWorkerModule() : ZMQTxModule()
    {
    }

    void event() override;
    void terminate() override;

  private:
    void createSocket() override;
    void proceedBroadcast() override;
  };
}