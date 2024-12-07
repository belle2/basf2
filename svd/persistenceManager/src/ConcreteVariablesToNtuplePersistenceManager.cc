#include <svd/persistenceManager/ConcreteVariablesToNtuplePersistenceManager.h>

#include <framework/utilities/MakeROOTCompatible.h>
#include <framework/utilities/RootFileCreationManager.h>
#include <framework/pcore/ProcHandler.h>

namespace {
  namespace Variables = Belle2::SVD::Variables;

  std::string typedVariableToLeafName(const Variables::TypedVariable& variable)
  {
    std::map<Variables::VariableDataType, std::string> suffix = {
      {Variables::VariableDataType::c_double, "/D"},
      {Variables::VariableDataType::c_int, "/I"},
      {Variables::VariableDataType::c_bool, "/O"}
    };

    std::string branchName = Belle2::MakeROOTCompatible::makeROOTCompatible(variable.getName());
    return branchName + suffix[variable.getDataType()];
  }
}

namespace Belle2::SVD {
  ConcreteVariablesToNtuplePersistenceManager::ConcreteVariablesToNtuplePersistenceManager()
    : m_tree{"", DataStore::c_Persistent}
  {

  }

  void ConcreteVariablesToNtuplePersistenceManager::initialize(const std::string& fileName,
      const std::string& treeName,
      const Variables::Variables& variables)
  {
    // TODO: Pass these in a constructor.
    m_fileName = fileName;
    m_treeName = treeName;

    if (m_fileName.empty()) {
      B2FATAL("Output root file name is not set.");
    }
    m_file = RootFileCreationManager::getInstance().getFile(m_fileName);

    if (not m_file) {
      B2FATAL("Could not create file: " << m_fileName << ".");
    }

    TDirectory::TContext directoryGuard{m_file.get()};

    if (m_file->Get(m_treeName.c_str())) {
      B2FATAL("A tree with the name: " << m_treeName << "already exists in the file: " << m_fileName << ".");
    }

    m_tree.registerInDataStore(m_fileName + m_treeName, DataStore::c_DontWriteOut);
    m_tree.construct(m_treeName.c_str(), "");
    m_tree->get().SetCacheSize(100000);

    // TODO: Set counter branches

    // Set branches
    // for (const auto& variable : variables) {
    //   registerBranch(variable);
    // }
    m_tree->get().SetBasketSize("*", m_basketSize);
  }

  void ConcreteVariablesToNtuplePersistenceManager::addEntry(const Variables::EvaluatedVariables& evaluatedVariables)
  {
    for (const auto& [variableName, value] : evaluatedVariables) {
      std::string branchName = Belle2::MakeROOTCompatible::makeROOTCompatible(variableName.c_str());
      updateBranch(branchName, value);
      m_tree->get().Fill();
    }
  }

  void ConcreteVariablesToNtuplePersistenceManager::store()
  {
    if (not ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
      B2INFO("Writing Ntuple" << m_treeName);
      TDirectory::TContext directoryGuard{m_file.get()};
      m_tree->write(m_file.get());

      const bool writeError = m_file->TestBit(TFile::kWriteError);
      m_file.reset();
      if (writeError) {
        B2FATAL("A write error occurred while saving '" << m_fileName << "', please check if enough disk space is available.");
      }
    }
  }

  void ConcreteVariablesToNtuplePersistenceManager::registerBranch(const Variables::Variable& variable)
  {
    const auto* typedVariable = dynamic_cast<const Variables::TypedVariable*>(&variable);

    std::string branchName = Belle2::MakeROOTCompatible::makeROOTCompatible(typedVariable->getName().c_str());
    std::string leafName = typedVariableToLeafName(*typedVariable);

    if (not typedVariable) {
      B2FATAL("...");
    }

    switch (typedVariable->getDataType()) {
      case Variables::VariableDataType::c_double:
        m_branchesDouble[branchName] = double{};
        m_tree->get().Branch(branchName.c_str(), &m_branchesDouble[branchName], leafName.c_str());
        break;
      case Variables::VariableDataType::c_int:
        m_branchesInt[branchName] = int{};
        m_tree->get().Branch(branchName.c_str(), &m_branchesInt[branchName], leafName.c_str());
        break;
      case Variables::VariableDataType::c_bool:
        m_branchesBool[branchName] = bool{};
        m_tree->get().Branch(branchName.c_str(), &m_branchesBool[branchName], leafName.c_str());
        break;
      default:
        break;
    }
  }

  void ConcreteVariablesToNtuplePersistenceManager::updateBranch(const std::string& branchName,
      const Variables::ReturnType& evaluatedValue)
  {
    std::visit([&](auto&& value) {
      using T = std::decay_t<decltype(value)>;

      if constexpr(std::is_same_v<T, double>) {
        m_branchesDouble[branchName] = value;
      } else if constexpr(std::is_same_v<T, int>) {
        m_branchesInt[branchName] = value;
      } else if constexpr(std::is_same_v<T, bool>) {
        m_branchesBool[branchName] = value;
      }
    }, evaluatedValue);
  }
}