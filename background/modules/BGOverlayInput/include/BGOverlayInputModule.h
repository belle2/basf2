/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Basf2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

/* ROOT headers. */
#include <TChain.h>

/* C++ headers. */
#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  /**
   * Beam BG data input, either in form of Digits or raw data.
   * Both kinds of data are kept at the DataStore in a separate StoreArrays
   */
  class BGOverlayInputModule : public Module {

  public:

    /**
     * Constructor
     */
    BGOverlayInputModule();

    /**
     * Destructor
     */
    ~BGOverlayInputModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    void beginRun() override;

    /**
     * Event processor.
     */
    void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    void terminate() override;

  private:

    /**
     * Register branches of the given event tree to the data store,
     * except EventMetaData and all relations
     *
     * @return True if the branches are registered successfully.
     */
    bool registerBranches();

    /**
     * Connect registered branches to the data store
     */
    void connectBranches();

    std::vector<std::string> m_inputFileNames; /**< list of file names */
    std::string m_extensionName; /**< name added to default branch names */
    std::string m_BackgroundInfoInstanceName = ""; /**< name of BackgroundInfo branch */
    bool m_skipExperimentCheck = false; /**< flag for skipping the check on the experiment number */
    bool m_ignoreRunNumbers = false; /**< flag for ignoring the run numbers in run-dependent MC */

    TChain* m_tree = 0;            /**< tree pointer */
    unsigned m_numEvents = 0;      /**< number of events (tree entries) in the sample */
    unsigned m_eventCount = 0;     /**< current event (tree entry) */
    unsigned m_firstEvent = 0;     /**< randomly chosen first event (tree entry) */
    bool m_start = true;           /**< flag denoting first call of event function (of each run if overlay is run-dependent) */
    std::vector<DataStore::StoreEntry*> m_storeEntries;  /**< store entries of registered branches */
    std::vector<std::string> m_branchNames; /**< names of registered branches */
    std::vector<std::string> m_otherBranchNames; /**< other branch names found in the tree */
    unsigned m_index = 0; /**< index of BackgroundDescr in BackgroundInfo object */

    StoreObjPtr<EventMetaData> m_eventMetaData; /**< event meta data */
    std::map<int, std::vector<std::string>> m_runFileNamesMap; /**< Map between runs and file names */
    bool m_runByRun = false; /**< internal flag for steering between run-independent (false) and run-dependent (true) overlay */
  };

} // Belle2 namespace

