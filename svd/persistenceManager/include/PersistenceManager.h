#pragma once

#include <string>
#include <vector>

#include <svd/persistenceManager/Types.h>

namespace Belle2::SVD {

  /**
   * @class PersistenceManager
   * @brief Abstract base class defining the interface for persisting SVD variables.
   *
   * Any class implementing this interface is responsible for storing and
   * retrieving SVD-related data (e.g., variables to ntuples or histograms).
   * Concrete implementations must provide the following capabilities:
   *   - Initialization of underlying data structures (e.g., files, trees, histograms)
   *   - Adding or accumulating event entries
   *   - Writing final results to disk or another storage medium
   */
  class PersistenceManager {
  public:
    /**
     * @brief Initializes the persistence manager with the given parameters.
     * @param fileName Name or path to the output resource (e.g., a ROOT file).
     * @param objectName Name of the object (e.g., tree or directory) used to store data.
     * @param variables A list of variables (TypedVariable or BinnedVariable) used by the manager.
     *
     * This function should set up any internal data structures needed,
     * such as creating file pointers, booking histograms, or initializing
     * trees. It does not actually store data but prepares for it.
     */
    virtual void initialize(const std::string& fileName,
                            const std::string& objectName,
                            Variables::Variables& variables) = 0;

    /**
     * @brief Adds a new data entry (event) to the underlying storage.
     * @param evaluatedVariables A map of variable names to their evaluated values.
     *
     * This method is called once per event or data set. The implementing class
     * should use the provided values to fill or update the data structures (e.g.,
     * histograms, tree branches, or other objects).
     */
    virtual void addEntry(const Variables::EvaluatedVariables& evaluatedVariables) = 0;

    /**
     * @brief Writes the current in-memory data to the final storage medium.
     *
     * This method should be called once the data accumulation is complete (e.g.,
     * at the end of a run). It ensures that all buffered data is safely written
     * to the output (e.g., a ROOT file).
     */
    virtual void store() = 0;

    /**
     * @brief Virtual destructor.
     */
    virtual ~PersistenceManager() = default;
  };

} // namespace Belle2::SVD
