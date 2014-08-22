/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPNTUPLEMODULE_H
#define TOPNTUPLEMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <top/modules/TOPNtuple/NtupleStructure.h>
#include <top/geometry/TOPGeometryPar.h>
#include "TFile.h"

namespace Belle2 {

  /**
   * Module to write out a ntuple summarizing TOP reconstruction output.
   * Used for the validation.
   */
  class TOPNtupleModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPNtupleModule();

    /**
     * Destructor
     */
    virtual ~TOPNtupleModule();

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

    std::string m_outputFileName; /**< output file name (root file) */

    TFile* m_file;      /**< TFile */
    TTree* m_tree;      /**< TTree with TOPTree structure */
    TOP::TOPTree m_top; /**< ntuple structure */

    TOP::TOPGeometryPar* m_topgp;  /**< geometry parameters */

  };

} // Belle2 namespace

#endif
