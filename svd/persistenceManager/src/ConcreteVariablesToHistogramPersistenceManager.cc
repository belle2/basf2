#include <svd/persistenceManager/ConcreteVariablesToHistogramPersistenceManager.h>
// #include <svd/variables/Variable.h>

#include <framework/logging/Logger.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/core/Environment.h>
#include <framework/utilities/MakeROOTCompatible.h>
#include <framework/utilities/RootFileCreationManager.h>

namespace Belle2::SVD {
  ConcreteVariablesToHistogramPersistenceManager::ConcreteVariablesToHistogramPersistenceManager() {}

  void ConcreteVariablesToHistogramPersistenceManager::initialize(const std::string& fileName,
      const std::string& directoryName,
      Variables::Variables& variables)
  {
    m_fileName = fileName;
    m_directory = directoryName;
    m_variables = variables;

    openFileWithGuards();
    registerHistograms();
  }

  void ConcreteVariablesToHistogramPersistenceManager::addEntry(const Variables::EvaluatedVariables& evaluatedVariables)
  {
    for (const auto& [variableName, value] : evaluatedVariables) {
      std::visit([&](auto&& val) {
        (*m_histograms[variableName])->get().Fill(val);
      }, value);
    }
  }

  void ConcreteVariablesToHistogramPersistenceManager::store()
  {
    if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
      TDirectory::TContext directoryGuard(m_file.get());
      if (not m_directory.empty()) {
        m_file->cd(m_directory.c_str());
      }
      B2INFO("Writing Histograms to " << gDirectory->GetPath());
      for (auto it = m_histograms.begin(); it != m_histograms.end(); ++it) {
        (*it->second)->write(gDirectory);
      }
      const bool writeError = m_file->TestBit(TFile::kWriteError);
      m_file.reset();
      if (writeError) {
        B2FATAL("A write error occurred while saving '" << m_fileName  << "', please check if enough disk space is available.");
      }
    }
  }

  void ConcreteVariablesToHistogramPersistenceManager::openFileWithGuards()
  {
    m_file = RootFileCreationManager::getInstance().getFile(m_fileName);

    if (!m_file) return;

    TDirectory::TContext directoryGuard(m_file.get());
    if (not m_directory.empty()) {
      m_directory = MakeROOTCompatible::makeROOTCompatible(m_directory);
      m_file->mkdir(m_directory.c_str());
      m_file->cd(m_directory.c_str());
    }
  }

  void ConcreteVariablesToHistogramPersistenceManager::registerHistograms()
  {
    for (const auto& variable : m_variables) {
      std::visit([&](const auto & typedVariable) {
        using T = std::decay_t<decltype(typedVariable)>;
        if constexpr(std::is_same_v<T, Variables::BinnedVariable>) {
          std::string varStr = typedVariable.getName();
          int varNbins = typedVariable.getNbins();
          float low = typedVariable.getLowBin();
          float high = typedVariable.getHighBin();

          std::string compatibleName = MakeROOTCompatible::makeROOTCompatible(varStr);

          auto ptr = std::make_unique<StoreObjPtr<RootMergeable<TH1D>>>("", DataStore::c_Persistent);
          ptr->registerInDataStore(m_fileName + m_directory + varStr, DataStore::c_DontWriteOut);
          ptr->construct(compatibleName.c_str(), compatibleName.c_str(), varNbins, low, high);

          m_histograms[compatibleName] = std::move(ptr);
        } else {
          B2WARNING("Incompatible variable type. Skipping histogram registration.");
        }
      }, variable);
    }
  }
}
