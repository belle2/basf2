/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>

/* ROOT headers. */
#include <TChain.h>

/* C++ headers. */
#include <string>

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

    typedef std::vector<DataStore::StoreEntry*> StoreEntries;   /**< DataStore entries */

    /**
     * Connect branches of the given event tree to the data store,
     * except EventMetaData and all relations
     *
     * @return True if the branches are connected successfully.
     */
    bool connectBranches();

    std::vector<std::string> m_inputFileNames; /**< list of file names */
    std::string m_extensionName; /**< name added to default branch names */
    std::string m_BackgroundInfoInstanceName = ""; /**< name BackgroundInfo name */
    bool m_skipExperimentCheck = false; /**< flag for skipping the check on the experiment number */

    TChain* m_tree = 0;            /**< tree pointer */
    unsigned m_numEvents = 0;      /**< number of events (tree entries) in the sample */
    unsigned m_eventCount = 0;     /**< current event (tree entry) */
    unsigned m_firstEvent = 0;     /**< randomly choosen first event (tree entry) */
    bool m_start = true;           /**< flag denoting first call of event function */
    StoreEntries m_storeEntries;   /**< store entries with connected branches */
    unsigned m_index = 0; /**< index of BackgroundDescr in BackgroundInfo object */

  };

} // Belle2 namespace

