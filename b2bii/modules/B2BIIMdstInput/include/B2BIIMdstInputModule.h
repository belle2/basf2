//+
// File : B2BIIMdstInputModule.h
// Description : A module to read Panther record in basf2 from Belle MDST file
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 16 - Feb - 2015
//-

#ifndef B2BIIMDST_INPUT_H
#define B2BIIMDST_INPUT_H

#include <framework/core/Module.h>

#define BELLE_TARGET_H "belle-x86_64-unknown-linux-gnu-g++.h"
#include "belle_legacy/panther/panther.h"
#include "belle_legacy/panther/panther_group.h"

#include <string>

namespace Belle2 {

  /**
   * Module reads Belle MDST files and converts Belle_event record
   * to Belle II EventMetaData StoreObjectPointer.
   */

  class B2BIIMdstInputModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    B2BIIMdstInputModule();
    virtual ~B2BIIMdstInputModule() override;

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    // Data members
  private:
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

    //! PantherFile
    Belle::Panther_FileIO* m_fd{nullptr};

    //! Event counter
    int m_nevt;

  };

} // end namespace Belle2

#endif
