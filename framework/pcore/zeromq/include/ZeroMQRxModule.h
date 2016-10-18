#pragma once

#include <framework/pcore/zeromq/ZeroMQModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/RandomGenerator.h>
#include <framework/pcore/DataStoreStreamer.h>

#include <framework/pcore/zeromq/ZeroMQMessage.h>

#include <zmq.hpp>

#include <memory.h>

namespace Belle2 {
  class ZeroMQRxModule : public ZeroMQModule {
  public:
    ZeroMQRxModule();

    void initialize() override
    {
      m_randomgenerator.registerInDataStore(DataStore::c_DontWriteOut);
    }
  };
}