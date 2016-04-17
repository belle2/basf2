/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TPCDATAROOTINPUTMODULE_H
#define TPCDATAROOTINPUTMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <beast/microtpc/modules/TpcTree.h>
#include "TFile.h"
#include "TChain.h"

namespace Belle2 {

  /**
   * Intput of LEPS 2013 test beam data from a specific root ntuple ("top tree")
   */
  class TpcDataRootInputModule : public Module {

  public:

    /**
     * Constructor
     */
    TpcDataRootInputModule();

    /**
     * Destructor
     */
    virtual ~TpcDataRootInputModule();

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

    /**
     * Prints module parameters.
     */
    void printModuleParams() const;

  private:

    std::vector<std::string> m_inputFileNames; /**< list of file names */
    TChain* m_tree = 0;            /**< tree pointer */
    std::string m_inputFileName;  /**< input file name */
    int m_inputTPCNumber;         /**<input TPC number readout */

    TFile* m_file;        /**< TFile */
    TTree* m_treeTpc;     /**< TTree data tpc */
    TTree* m_treeTpcPar;  /**< TTree tpc parameters */
    TPC::TpcTree m_tpc;   /**< TPC tree structure */

    int m_numEntries;     /**< number of ntuple entries */
    int m_entryCounter;   /**< entry counter */
    unsigned m_numEvents = 0;      /**< number of events (tree entries) in the sample */
    unsigned m_eventCount = 0;     /**< current event (tree entry) */
    unsigned m_exp = 0;            /**< Date of the day */
  };

} // Belle2 namespace

#endif
