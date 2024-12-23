#pragma once

#include <svd/persistenceManager/PersistenceManager.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/RootMergeable.h>

namespace Belle2::SVD {
  class ConcreteVariablesToHistogramPersistenceManager : public PersistenceManager {
  public:
    ConcreteVariablesToHistogramPersistenceManager();

    void initialize(const std::string&, const std::string&, Variables::Variables&) override;
    void addEntry(const Variables::EvaluatedVariables&) override;
    void store() override;

  private:
    std::string m_fileName;
    std::string m_directory;
    Variables::Variables m_variables;

    std::shared_ptr<TFile> m_file{nullptr};
    std::map<std::string, std::unique_ptr<StoreObjPtr<RootMergeable<TH1D>>>> m_histograms;

    void openFileWithGuards();
    void registerHistograms();
  };
}