#pragma once

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
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
    virtual void writeEvent(const std::unique_ptr<ZMQNoIdMessage>& message);

  private:
    StoreObjPtr<EventMetaData> m_eventMetaData;

    void createSocket() override;
    void proceedMulticast() override;

    // TODO: received end messages from all alive worker
    //int m_numberOfAliveWorkers = 0;
  };
}