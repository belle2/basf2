#pragma once

#include <framework/pcore/zmq/processModules/ZMQModule.h>

namespace Belle2 {
  class ZMQTxModule : public ZMQModule {
  public:
    ZMQTxModule();

    virtual void initialize() override;

  protected:
    void setRandomState();

  protected:
    bool m_param_sendRandomState;
  };
}