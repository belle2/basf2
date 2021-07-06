/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : B2BIIMdstInputModule.h
// Description : A module to read Panther record in basf2 from Belle MDST file
//
//-

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/FileMetaData.h>

#define BELLE_TARGET_H "belle-x86_64-unknown-linux-gnu-g++.h"
#include "belle_legacy/panther/panther.h"
#include "belle_legacy/panther/panther_group.h"

#include <string>
#include <set>

namespace Belle2 {

  /**
   * Module reads Belle MDST files and converts Belle_event record
   * to Belle II EventMetaData StoreObjectPointer.
   */

  class B2BIIMdstInputModule : public Module {

    // Public functions
  public:

    /** Constructor */
    B2BIIMdstInputModule();
    /** Destructor */
    virtual ~B2BIIMdstInputModule() override;

    /** Initialize the module */
    virtual void initialize() override;

    /** Called when a new run is started */
    virtual void beginRun() override;
    /** Called for each event */
    virtual void event() override;
    /** Called when the current run finished */
    virtual void endRun() override;
    /** Terminates the module. */
    virtual void terminate() override;

    // Data members
  private:

    StoreObjPtr<EventMetaData> m_evtMetaData; /**< event meta data Object pointer */

    /** Get list of input files, taking -i command line overrides into account. */
    std::vector<std::string> getInputFiles() const;

    /** Open the next file from the list of files
     * @returns true on success */
    bool openNextFile();
    /** Read the next event from the currently open file.
     * @returns true on success */
    bool readNextEvent();

    /**
     * Initializes Belle II DataStore.
     */
    void initializeDataStore();

    //-----------------------------------------------------------------------------
    // MISC
    //-----------------------------------------------------------------------------

    /** Input MDST file name for backwards compatibility */
    std::string m_inputFileName;
    /** List of input MDST filenames */
    std::vector<std::string> m_inputFileNames;
    /**
     * The number sequences (e.g. 23:42,101) defining the entries which are processed for each inputFileName.
     */
    std::vector<std::string> m_entrySequences;

    //! PantherFile
    Belle::Panther_FileIO* m_fd{nullptr};

    //! Global event counter
    int m_nevt;

    //! File counter
    int m_current_file_position;

    //! Entry counter in current file
    int m_current_file_entry;

    //! Entries numbers which are valid for the current file
    std::set<int64_t> m_valid_entries_in_current_file;

    //! File metadata to declare if we process real data or simulated data
    StoreObjPtr<FileMetaData> m_fileMetadata{"", DataStore::c_Persistent};
  };

} // end namespace Belle2
