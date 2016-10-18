#pragma once

#include <framework/pcore/SeqFile.h>

#include <string>
#include <memory.h>

namespace Belle2 {
  class DataStoreInitialization {
  public:
    static void initializeDataStore(const std::string& m_param_inputFileName);
    static void initializeDataStore(const std::unique_ptr<SeqFile>& file);
  };
}