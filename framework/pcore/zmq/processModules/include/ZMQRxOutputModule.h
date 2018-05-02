#pragma once
#include <framework/pcore/zmq/processModules/ZMQRxModule.h>

namespace Belle2 {
  class ZMQRxOutputModule : public ZMQRxModule {
  public:
    ZMQRxOutputModule() :
      ZMQRxModule()
    {
    }

    void event() override;

  protected:
    virtual void writeEventToDataStore(const std::unique_ptr<ZMQNoIdMessage>& message);

  private:
    void createSocket() override;
    void proceedBroadcast() override;

    //int m_numberOfAliveWorkers = 0;
  };
}