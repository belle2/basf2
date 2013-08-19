/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPLEPS2013INPUTMODULE_H
#define TOPLEPS2013INPUTMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <top/geometry/TOPGeometryPar.h>
#include <testbeam/top/modules/TOPLeps2013Output/TopTree.h>
#include "TFile.h"

namespace Belle2 {

  /**
   * Intput of LEPS 2013 test beam data from a specific root ntuple ("top tree")
   */
  class TOPLeps2013InputModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPLeps2013InputModule();

    /**
     * Destructor
     */
    virtual ~TOPLeps2013InputModule();

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

    std::string m_inputFileName;  /**< input file name */

    TFile* m_file;      /**< TFile */
    TTree* m_treeTop;   /**< TTree top */
    TOP::TopTree m_top; /**< top tree structure */

    TOP::TOPGeometryPar* m_topgp; /**< geometry parameters */

    int m_numPMTchannels; /**< number of PMT channels */
    int m_numChannels;    /**< number of all channels */
    float m_tdcWidth;     /**< TDC bin width [ps] */
    int m_tdcOverflow;    /**< TDC overflow value */
    int m_numEntries;     /**< number of ntuple entries */
    int m_entryCounter;   /**< entry counter */

  };

} // Belle2 namespace

#endif
