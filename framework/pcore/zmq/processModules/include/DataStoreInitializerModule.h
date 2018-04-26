#pragma once
#include <framework/core/Module.h>
#include <framework/pcore/SeqFile.h>

#include <string>

#include <sys/time.h>

namespace Belle2 {
  class DataStoreInitializerModule : public Module {
  public:
    DataStoreInitializerModule();

    void initialize() override;
  private:
    std::string m_param_inputFileName;
  };
}