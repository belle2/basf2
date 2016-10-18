#pragma once

#include <framework/pcore/zeromq/ZeroMQRxModule.h>
#include <framework/pcore/zeromq/RandomNameGenerator.h>
#include <framework/pcore/zeromq/DataStoreInitialization.h>

namespace Belle2 {
  class ZeroMQRxInputModule : public ZeroMQRxModule {
  public:
    ZeroMQRxInputModule() :
      ZeroMQRxModule()
    {
      addParam("initializeFileName", m_param_initializeFileName, "TODO", m_param_initializeFileName);

      // Randomize a new unique ID
      m_uniqueID = random_name(10);
    }

    void initialize() override
    {
      ZeroMQRxModule::initialize();

      if (not m_param_initializeFileName.empty()) {
        DataStoreInitialization::initializeDataStore(m_param_initializeFileName);
      }
    }

    void event() override;

  private:
    unsigned int m_bufferSize = 30;
    std::string m_param_initializeFileName = "";
    std::string m_uniqueID = "";

    void createSocket() override;
  };
}
