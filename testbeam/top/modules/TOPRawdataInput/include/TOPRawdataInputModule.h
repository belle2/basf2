/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPRAWDATAINPUTMODULE_H
#define TOPRAWDATAINPUTMODULE_H

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /**
   * Raw data reader for TOP beam tests
   */
  class TOPRawdataInputModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPRawdataInputModule();

    /**
     * Destructor
     */
    virtual ~TOPRawdataInputModule();

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

    /**
     * Open the raw data file
     * @params fileName file name
     * @return true on success
     */
    bool openFile(const std::string& fileName);

    /**
     * Close the raw data file
     */
    void closeFile();

    /**
     * Read data record.
     * Return status:
     *  - 0 on success (event can be further processed)
     *  - negative value for EOF or for corrupted file (file has to be closed)
     *  - positive value for non-event or corrupted record (try with next data record)
     * @return error status
     */
    int readDataRecord();

    std::string m_inputFileName; /**< input file name (raw data) */
    std::vector<std::string> m_inputFileNames; /**< list of input file names (raw data) */
    unsigned int m_listIndex;  /**< current index of input file name list element */
    unsigned long m_evtNumber; /**< current event number */
    unsigned long m_runNumber; /**< current run number */
    unsigned long m_expNumber; /**< current experiment number */
    unsigned m_wordsRead;      /**< number of words read from file */

  };

} // Belle2 namespace

#endif
