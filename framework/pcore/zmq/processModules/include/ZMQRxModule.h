#pragma once

#include <framework/pcore/zmq/processModules/ZMQModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/RandomGenerator.h>
#include <framework/pcore/DataStoreStreamer.h>

#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>

#include <zmq.hpp>

#include <memory.h>

namespace Belle2 {
  class ZMQRxModule : public ZMQModule {
  public:
    ZMQRxModule();

    void initialize() override
    {
      m_randomgenerator.registerInDataStore(DataStore::c_DontWriteOut);
    }
  };
}