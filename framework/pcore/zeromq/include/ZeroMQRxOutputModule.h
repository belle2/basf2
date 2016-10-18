#pragma once
#include <framework/pcore/zeromq/ZeroMQRxModule.h>

namespace Belle2 {
  class ZeroMQRxOutputModule : public ZeroMQRxModule {
  public:
    ZeroMQRxOutputModule() :
      ZeroMQRxModule()
    {
    }

    void event() override;

  protected:
    virtual void writeEvent(const std::unique_ptr<ZeroMQMessage>& message);

  private:
    void createSocket() override;

    int m_numberOfAliveWorkers = 0;
  };
}