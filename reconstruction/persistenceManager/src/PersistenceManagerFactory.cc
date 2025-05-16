#include <memory>
#include <framework/logging/Logger.h>
#include <reconstruction/persistenceManager/PersistenceManagerFactory.h>
#include <reconstruction/persistenceManager/ConcreteVariablesToNtuplePersistenceManager.h>
#include <reconstruction/persistenceManager/ConcreteVariablesToHistogramPersistenceManager.h>

namespace Belle2::VariablePersistenceManager {

  std::unique_ptr<PersistenceManager> PersistenceManagerFactory::create(const std::string& storageType)
  {
    if ("ntuple" == storageType) {
      return std::make_unique<ConcreteVariablesToNtuplePersistenceManager>();
    } else if ("histogram" == storageType) {
      return std::make_unique<ConcreteVariablesToHistogramPersistenceManager>();
    } else {
      B2ERROR("Incorrect storage type.");
      return nullptr;
    }
  }
}
