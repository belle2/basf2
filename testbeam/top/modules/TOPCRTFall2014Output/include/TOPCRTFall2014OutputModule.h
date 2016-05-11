/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Matthew Barrett                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <testbeam/top/modules/TOPCRTFall2014Output/CRT2014Tree.h>
#include "TFile.h"

namespace Belle2 {

  /**
   * Output of TOP Cosmic test fall 2014 test simulation to a specific root ntuple with
   * a data format matching parameters used in data root files ("top tree")
   */
  class TOPCRTFall2014OutputModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPCRTFall2014OutputModule();

    /**
     * Destructor
     */
    virtual ~TOPCRTFall2014OutputModule();

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

    std::string m_outputFileName; /**< Output file name */
    bool m_randomize;             /**< Randomize within TDC bin */
    double m_t0;   /**< Offset time to add for adjusting photon TDC times */
    bool m_multipleHits; /**< Allow multiple hits in a single channel within an event */

    TFile* m_file;      /**< TFile */
    TTree* m_treeTop;   /**< TTree top */
    TOP::TopTree m_top; /**< A struct containin the top tree structure */

    int m_numPMTchannels; /**< number of PMT channels */
    int m_numChannels;    /**< number of all channels */
    float m_tdcWidth;     /**< TDC bin width [ps] */
    int m_tdcOverflow;    /**< TDC overflow value */

  };

} // Belle2 namespace

