#pragma once

#include <reconstruction/persistenceManager/PersistenceManager.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/RootMergeable.h>

namespace Belle2::VariablePersistenceManager {

  /**
   * @class ConcreteVariablesToHistogramPersistenceManager
   * @brief A PersistenceManager that writes variables to ROOT histograms (TH1D).
   *
   * This class creates and manages one-dimensional histograms for a set of variables.
   * It handles opening a ROOT file, booking the histograms, and filling them with
   * user-supplied data each event. The histograms can be merged in a multi-process
   * environment through the RootMergeable mechanism.
   */
  class ConcreteVariablesToHistogramPersistenceManager : public PersistenceManager {
  public:
    /**
     * @brief Default constructor.
     */
    ConcreteVariablesToHistogramPersistenceManager();

    /**
     * @brief Initializes the manager by opening a ROOT file and creating histograms.
     * @param fileName The name of the ROOT file to create or update.
     * @param directory The directory (folder) within the ROOT file where histograms will be placed.
     * @param variables A list of variables that will be associated with the histograms.
     *
     * This method configures the persistence manager with file/directory names and prepares
     * the memory structures for histogram creation. It does not fill any data yet; that
     * is handled by @c addEntry().
     */
    void initialize(const std::string& fileName,
                    const std::string& directory,
                    Variables& variables) override;

    /**
     * @brief Fills histograms with the current set of evaluated variables.
     * @param evaluatedVariables A map of variable names to their current values.
     *
     * Each call updates the corresponding histogram with the provided variable values.
     */
    void addEntry(const EvaluatedVariables& evaluatedVariables) override;

    /**
     * @brief Writes histogram data to disk.
     *
     * After filling the histograms through @c addEntry(), this function ensures the
     * data is written (or merged) into the ROOT file.
     */
    void store() override;

  private:
    /**
     * @brief Name of the ROOT file where histograms will be written.
     */
    std::string m_fileName;

    /**
     * @brief Name of the directory (folder) within the ROOT file where histograms are stored.
     */
    std::string m_directory;

    /**
     * @brief A list of variables associated with the histograms.
     */
    Variables m_variables;

    /**
     * @brief Pointer to the ROOT file object.
     */
    std::shared_ptr<TFile> m_file{nullptr};

    /**
     * @brief A map of histogram names to their respective objects (wrapped in RootMergeable).
     */
    std::map<std::string, std::unique_ptr<StoreObjPtr<RootMergeable<TH1D>>>> m_histograms;

    /**
     * @brief Safely opens the ROOT file specified by @c m_fileName.
     *
     * Throws an exception if the file cannot be opened for writing or if
     * any other file-related error occurs.
     */
    void openFileWithGuards();

    /**
     * @brief Registers (books) the histograms based on @c m_variables.
     *
     * For each variable that is defined as "binned" (i.e., has bin information),
     * a TH1D histogram is created in @c m_histograms.
     */
    void registerHistograms();
  };

} // namespace Belle2::VariablePersistenceManager
