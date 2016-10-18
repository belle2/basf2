#pragma once

#include <framework/pcore/zeromq/ZeroMQTxModule.h>

namespace Belle2 {
  class ZeroMQTxOutputModule : public ZeroMQTxModule {
  public:
    ZeroMQTxOutputModule() : ZeroMQTxModule()
    {
    }

    void event() override;
    void terminate() override;

  private:
    void createSocket() override;
  };
}