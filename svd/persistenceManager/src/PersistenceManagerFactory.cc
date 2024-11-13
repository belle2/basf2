#include <memory>
#include <framework/logging/Logger.h>
#include <svd/persistenceManager/PersistenceManagerFactory.h>
#include <svd/persistenceManager/ConcreteVariablesToNtuplePersistenceManager.h>
#include <svd/persistenceManager/ConcreteVariablesToHistogramPersistenceManager.h>

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
