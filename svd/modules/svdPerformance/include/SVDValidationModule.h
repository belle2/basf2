/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
// #include <reconstruction/persistenceManager/PersistenceManager.h>

/**
 * @struct Counters
 * @brief Holds various counters for tracking event, run, and cluster data.
 */
struct Counters {
  int event;         /**< Event number */
  int run;           /**< Run number */
  int experiment;    /**< Experiment number */
  int production;    /**< Production number */
  int track;         /**< Track number */
  int cluster;       /**< Cluster number */
  unsigned int nClusters; /**< Total number of clusters */
};

namespace Belle2::SVD {

  /**
   * @class SVDValidationModule
   * @brief A module for validating SVD data. Outputs diagnostic information and stores data to file.
   *
   * This module reads SVD-related data (e.g., clusters) and performs basic validation or
   * analysis. It can write results to an output file, produce histograms for variables,
   * and store data in an ntuple for further examination.
   */
  class SVDValidationModule : public Module {
  public:
    /**
     * @brief Constructs the SVDValidationModule and sets default parameter values.
     */
    SVDValidationModule();

    /**
     * @brief Destructor (default).
     */
    virtual ~SVDValidationModule() = default;

    /**
     * @brief Initializes the module. Called once before the main event loop.
     *
     * This function is used to set up file outputs, book histograms, and
     * prepare other resources required during event processing.
     */
    virtual void initialize() override;

    /**
     * @brief Processes each event in the main event loop.
     *
     * This function is called once per event. It retrieves SVD data (e.g., clusters),
     * updates counters, and fills histograms or ntuples with the relevant variables.
     */
    virtual void event() override;

    /**
     * @brief Performs finalization steps after all events have been processed.
     *
     * Called once after all events are processed to close files, save histograms, or
     * release resources allocated during the module run.
     */
    virtual void terminate() override;

  private:
    std::string m_fileName;          /**< The output file name for storing results (e.g., root file). */
    std::string m_containerName;     /**< The name of the container storing SVD data (e.g., clusters). */
    std::string m_particleListName;  /**< The name of the particle list to use for analysis (if applicable). */

    std::vector<std::string> m_variablesToNtuple;  /**< List of variables to store in an ntuple. */
    std::vector<std::tuple<std::string, int, float, float>>
                                                         m_variablesToHistogram; /**< List of (variableName, nBins, lowBin, highBin) defining histograms. */
    std::vector<std::string> m_variableNames;  /**< List of variable names for reference. */

    // std::unique_ptr<PersistenceManager> persistenceManager; /**< Manages the persistence of data (e.g., writing to file). */
  };

} // namespace Belle2::SVD