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

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <string>
#include "TChain.h"

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
    virtual ~BGOverlayInputModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

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

    TChain* m_tree = 0;            /**< tree pointer */
    unsigned m_numEvents = 0;      /**< number of events (tree entries) in the sample */
    unsigned m_eventCount = 0;     /**< current event (tree entry) */
    unsigned m_firstEvent = 0;     /**< randomly choosen first event (tree entry) */
    bool m_start = true;           /**< flag denoting first call of event function */
    StoreEntries m_storeEntries;   /**< store entries with connected branches */
    unsigned m_index = 0; /**< index of BackgroundDescr in BackgroundInfo object */

  };

} // Belle2 namespace

