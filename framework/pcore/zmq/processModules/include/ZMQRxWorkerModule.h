#pragma once

#include <framework/pcore/zmq/processModules/ZMQRxModule.h>
#include <framework/pcore/zmq/processModules/RandomNameGenerator.h>
#include <framework/pcore/zmq/processModules/DataStoreInitialization.h>



namespace Belle2 {
  class ZMQRxWorkerModule : public ZMQRxModule {
  public:
    ZMQRxWorkerModule() :
      ZMQRxModule()
    {
      addParam("initializeFileName", m_param_initializeFileName, "TODO", m_param_initializeFileName);
    }

    void initialize() override
    {
      ZMQRxModule::initialize();

      if (not m_param_initializeFileName.empty()) {
        DataStoreInitialization::initializeDataStore(m_param_initializeFileName);
      }
    }

    void event() override;

  private:
    unsigned int m_bufferSize = 30;
    unsigned int m_timeout = 10;
    std::string m_param_initializeFileName = "";

    void createSocket() override;
    void proceedMulticast() override;
  };
}