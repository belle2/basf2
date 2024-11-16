#pragma once

#include <svd/persistenceManager/PersistenceManager.h>

namespace Belle2::SVD {
  class PersistenceManagerFactory {
  public:
    static std::unique_ptr<PersistenceManager> create(const std::string& storageType);
  };
}