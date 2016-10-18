#pragma once

#include <framework/pcore/zeromq/ZeroMQModule.h>

namespace Belle2 {
  class ZeroMQTxModule : public ZeroMQModule {
  public:
    ZeroMQTxModule();

    virtual void initialize() override;

  protected:
    void setRandomState();

  protected:
    bool m_param_sendRandomState;
  };
}