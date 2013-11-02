/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPLEPS2013OUTPUTMODULE_H
#define TOPLEPS2013OUTPUTMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <top/geometry/TOPGeometryPar.h>
#include <testbeam/top/modules/TOPLeps2013Output/TopTree.h>
#include "TFile.h"

namespace Belle2 {

  /**
   * Output of LEPS 2013 test beam simulation to a specific root ntuple with
   * the same data format as used for the measured data ("top tree")
   */
  class TOPLeps2013OutputModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPLeps2013OutputModule();

    /**
     * Destructor
     */
    virtual ~TOPLeps2013OutputModule();

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

    std::string m_outputFileName; /**< output file name */
    bool m_randomize;             /**< randomize within TDC bin */
    double m_t0;   /**< offset time to add for adjusting photon TDC times */

    TFile* m_file;      /**< TFile */
    TTree* m_treeTop;   /**< TTree top */
    TOP::TopTree m_top; /**< top tree structure */

    TOP::TOPGeometryPar* m_topgp; /**< geometry parameters */

    int m_numPMTchannels; /**< number of PMT channels */
    int m_numChannels;    /**< number of all channels */
    float m_tdcWidth;     /**< TDC bin width [ps] */
    int m_tdcOverflow;    /**< TDC overflow value */

  };

} // Belle2 namespace

#endif
