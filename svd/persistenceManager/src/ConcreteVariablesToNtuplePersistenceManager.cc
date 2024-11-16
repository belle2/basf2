#include <svd/persistenceManager/ConcreteVariablesToNtuplePersistenceManager.h>
// #include <svd/variables/Variable.h>

namespace Belle2::SVD {
  ConcreteVariablesToNtuplePersistenceManager::ConcreteVariablesToNtuplePersistenceManager()
    : m_tree{"", DataStore::c_Persistent}
  {

  }

  void ConcreteVariablesToNtuplePersistenceManager::initialize(const std::string& fileName,
      const std::string& treeName,
      const Variables::ComputableVariables& variables)
  {
    // // TODO: Move some of the logic to the constructor?
    // m_fileName = fileName;
    // m_treeName = treeName;
    // m_variables = variables;

    // m_file = RootFileCreationManager::getInstance().getFile(m_fileName);
    // TDirectory::TContext directoryGuard{m_file.get()};

    // m_tree.registerInDataStore(m_fileName + m_treeName, DataStore::c_DontWriteOut);
    // m_tree.construct(m_treeName.c_str(), "");
    // m_tree->get().SetCacheSize(100000);

    // // Set branches
    // // TODO: Maybe this can be moved to addEntry with a guard that we set a branch if it hasn't been set already
    // for (variable : m_variables)
    // {
    //   // TODO: Improve this nasty operator, add some kind of converter and consider other
    //   // variable types as well.
    //   std::string suffix = variable.type == variable.VariableType::c_int ? "/I" : "/D";

    //   m_tree->get().Branch(variable.name.c_str(), &m_branches[variable.name], (variable.name+suffix).c_str());
    // }

    // TODO: Set basket size, whatever that is
  }

  void ConcreteVariablesToNtuplePersistenceManager::addEntry(const Variables::EvaluatedVariables& variables)
  {
    //
    // m_tree->get().Fill();
  }

  void ConcreteVariablesToNtuplePersistenceManager::store()
  {

  }
}