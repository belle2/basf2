#pragma once

#include <memory>
#include <map>
#include <string>
#include <TTree.h>
#include <TFile.h>

#include <svd/persistenceManager/PersistenceManager.h>
#include <framework/pcore/RootMergeable.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2::SVD {
  class ConcreteVariablesToNtuplePersistenceManager : public PersistenceManager {
  public:
    ConcreteVariablesToNtuplePersistenceManager();

    void initialize(const std::string&, const std::string&, const Variables::ComputableVariables&) override;
    void addEntry(const Variables::EvaluatedVariables&) override;
    void store() override;

  private:
    std::string m_fileName;
    std::string m_treeName;
    std::string m_variables;

    std::shared_ptr<TFile> m_file{nullptr};
    StoreObjPtr<RootMergeable<TTree>> m_tree;
    std::map<std::string, std::variant<int, double>> m_branches;
  };
}
