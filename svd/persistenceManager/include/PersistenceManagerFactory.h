#pragma once

#include <svd/persistenceManager/PersistenceManager.h>

class PersistenceManagerFactory {
public:
  static std::unique_ptr<PersistenceManager> create(const std::string& storageType);
};
