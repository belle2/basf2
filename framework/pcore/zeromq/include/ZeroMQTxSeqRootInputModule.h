#pragma once

#include <framework/pcore/zeromq/ZeroMQTxInputModule.h>
#include <framework/pcore/zeromq/DataStoreInitialization.h>
#include <framework/dataobjects/EventMetaData.h>
#include <deque>

namespace Belle2 {
  class ZeroMQTxSeqRootInputModule : public ZeroMQTxInputModule {
  public:
    ZeroMQTxSeqRootInputModule() :
      ZeroMQTxInputModule()
    {
      addParam("inputFileName", m_param_inputFileName, "File name of the file to read the events from.");
    }

    void initialize() override
    {
      ZeroMQTxInputModule::initialize();
      m_seqRootFile.reset(new SeqFile(m_param_inputFileName.c_str(), "r"));

      DataStoreInitialization::initializeDataStore(m_seqRootFile);
    }

  protected:
    std::unique_ptr<ZeroMQRoutedMessage> readEvent() override;

  private:
    std::string m_param_inputFileName;
    std::unique_ptr<SeqFile> m_seqRootFile;

    StoreObjPtr<EventMetaData> m_eventMetaData;
  };
}