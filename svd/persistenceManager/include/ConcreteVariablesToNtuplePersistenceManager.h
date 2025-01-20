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

    void initialize(const std::string&, const std::string&, Variables::Variables&) override;
    void addEntry(const Variables::EvaluatedVariables&) override;
    void store() override;

  private:
    std::string m_fileName;
    std::string m_treeName;
    Variables::Variables m_variables;

    int m_basketSize{1600};

    std::shared_ptr<TFile> m_file{nullptr};
    StoreObjPtr<RootMergeable<TTree>> m_tree;

    std::map<std::string, double> m_branchesDouble;
    std::map<std::string, int> m_branchesInt;
    std::map<std::string, bool> m_branchesBool;

    void openFileWithGuards();
    void registerBranches();
    void updateBranch(const std::string&, const Variables::VariableType&);
  };
}
