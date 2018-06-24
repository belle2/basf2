#pragma once

#include <framework/pcore/zmq/processModules/ZMQRxModule.h>
#include <framework/pcore/zmq/processModules/RandomNameGenerator.h>



namespace Belle2 {
  class ZMQRxWorkerModule : public ZMQRxModule {
  public:


    void event() override;
    void terminate() override;
    void proceedMulticast() override {}

  private:
    unsigned int m_bufferSize = 2;

    void createSocket() override;
  };
}