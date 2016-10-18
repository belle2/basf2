#pragma once
#include <framework/pcore/zeromq/ZeroMQRxOutputModule.h>
#include <framework/pcore/zeromq/DataStoreInitialization.h>
#include <framework/pcore/SeqFile.h>
#include <framework/dataobjects/EventMetaData.h>

namespace Belle2 {
  class ZeroMQRxSeqRootOutputModule : public ZeroMQRxOutputModule {
  public:
    ZeroMQRxSeqRootOutputModule() :
      ZeroMQRxOutputModule()
    {
      addParam("outputFileName", m_param_outputFileName, "File name of the output file to write to.");
      addParam("initializeFileName", m_param_initializeFileName, "TODO", m_param_initializeFileName);
    }

    void initialize() override
    {
      m_eventMetaData.registerInDataStore();

      m_seqRootFile.reset(new SeqFile(m_param_outputFileName.c_str(), "w"));

      if (not m_param_initializeFileName.empty()) {
        DataStoreInitialization::initializeDataStore(m_param_initializeFileName);
      }
    }


  protected:
    void writeEvent(const std::unique_ptr<ZeroMQMessage>& message) override;

  private:
    std::string m_param_outputFileName;
    std::string m_param_initializeFileName = "";
    std::unique_ptr<SeqFile> m_seqRootFile;

    StoreObjPtr<EventMetaData> m_eventMetaData;

    void writeStreamerInfo();
  };
}