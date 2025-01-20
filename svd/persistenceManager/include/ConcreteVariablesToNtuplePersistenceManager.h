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

  /**
   * @class ConcreteVariablesToNtuplePersistenceManager
   * @brief A PersistenceManager that writes variables to a ROOT TTree.
   *
   * This class manages the creation and population of a ROOT TTree for a given set
   * of variables. It handles opening a ROOT file, booking TTree branches, and filling
   * them with user-supplied data each event. The TTree can then be merged in a
   * multi-process environment through the RootMergeable mechanism.
   */
  class ConcreteVariablesToNtuplePersistenceManager : public PersistenceManager {
  public:
    /**
     * @brief Default constructor.
     */
    ConcreteVariablesToNtuplePersistenceManager();

    /**
     * @brief Initializes the manager by opening a ROOT file and preparing a TTree.
     * @param fileName Name of the ROOT file to create or update.
     * @param treeName Name of the TTree to create.
     * @param variables List of variables that will be stored in the TTree.
     *
     * This method configures the persistence manager with file and tree names,
     * and sets up the necessary memory structures. It does not actually write data;
     * that is handled by @c addEntry().
     */
    void initialize(const std::string& fileName,
                    const std::string& treeName,
                    Variables::Variables& variables) override;

    /**
     * @brief Adds a single event's worth of variable data to the TTree.
     * @param evaluatedVariables Map of variable names to their values, which will be
     *        written to the TTree branches.
     *
     * Each call updates the branch buffers for the registered variables. The actual
     * storage of these buffers occurs when @c store() is called.
     */
    void addEntry(const Variables::EvaluatedVariables& evaluatedVariables) override;

    /**
     * @brief Writes the current buffered data to disk.
     *
     * After populating branch buffers through @c addEntry(), this function commits
     * them to the ROOT file, ensuring data is safely stored.
     */
    void store() override;

  private:
    /**
     * @brief Name of the ROOT file where TTree data is stored.
     */
    std::string m_fileName;

    /**
     * @brief Name of the TTree that will be created and filled.
     */
    std::string m_treeName;

    /**
     * @brief The list of variables that will be written to the TTree.
     */
    Variables::Variables m_variables;

    /**
     * @brief Basket size for the TTree branches. Affects I/O efficiency.
     */
    int m_basketSize{1600};

    /**
     * @brief Pointer to the ROOT file object.
     */
    std::shared_ptr<TFile> m_file{nullptr};

    /**
     * @brief A store pointer to the RootMergeable wrapper for the TTree.
     */
    StoreObjPtr<RootMergeable<TTree>> m_tree;

    /**
     * @brief Maps of variable names to storage for double, int, and bool TTree branches.
     */
    std::map<std::string, double> m_branchesDouble; /**< Storage for double branches. */
    std::map<std::string, int>    m_branchesInt;    /**< Storage for int branches. */
    std::map<std::string, bool>   m_branchesBool;   /**< Storage for bool branches. */

    /**
     * @brief Safely opens the ROOT file specified by @c m_fileName.
     *
     * This function takes care of ensuring the file can be written to
     * or created if it does not exist. Throws an exception on error.
     */
    void openFileWithGuards();

    /**
     * @brief Registers TTree branches for each variable in @c m_variables.
     *
     * Initializes branch addresses for each variable type (double, int, bool).
     * Also sets the basket size using @c m_basketSize.
     */
    void registerBranches();

    /**
     * @brief Updates the branch buffer for a given variable with a new value.
     * @param variableName The name of the variable (branch).
     * @param value The new value to be stored in the branch buffer.
     *
     * Depending on the type of the variant, the appropriate map
     * (m_branchesDouble, m_branchesInt, or m_branchesBool) is updated.
     */
    void updateBranch(const std::string& variableName,
                      const Variables::VariableType& value);
  };

} // namespace Belle2::SVD
