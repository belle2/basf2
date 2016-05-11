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
#include <string>
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

  private:

    /**
     * Convert new numbering scheme for pixels to the old one
     * @param pixelID pixel ID in the new numbering scheme
     * @return pixel ID in the old numbering scheme
     */
    int getOldNumbering(int pixelID) const;

    std::string m_outputFileName; /**< output file name */
    bool m_randomize;             /**< randomize within TDC bin */
    double m_t0;   /**< offset time to add for adjusting photon TDC times */
    bool m_multipleHits; /**< Allow multiple hits in a single channel within an event */

    TFile* m_file;      /**< TFile */
    TTree* m_treeTop;   /**< TTree top */
    TOP::TopTree m_top; /**< top tree structure */

    int m_numPMTchannels; /**< number of PMT channels */
    int m_numChannels;    /**< number of all channels */
    float m_tdcWidth;     /**< TDC bin width [ps] */
    int m_tdcOverflow;    /**< TDC overflow value */

  };

} // Belle2 namespace

