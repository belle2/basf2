#pragma once

#include <svd/persistenceManager/PersistenceManager.h>

namespace Belle2::SVD {

  /**
   * @class PersistenceManagerFactory
   * @brief A factory class that creates specific PersistenceManager objects.
   *
   * This factory allows clients to create different concrete PersistenceManager
   * implementations by specifying a storage type (e.g., histogram, ntuple, etc.).
   */
  class PersistenceManagerFactory {
  public:
    /**
     * @brief Creates a PersistenceManager instance based on the specified storage type.
     * @param storageType A string indicating the type of storage (e.g., "ntuple", "histogram").
     * @return A unique_ptr to a newly created PersistenceManager object.
     *
     * The exact implementation (concrete class) returned depends on
     * the provided storageType. If the type is not recognized,
     * this function may return a null pointer or throw an exception
     * (depending on the implementation).
     */
    static std::unique_ptr<PersistenceManager> create(const std::string& storageType);
  };

} // namespace Belle2::SVD
