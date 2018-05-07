/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <top/reconstruction/TOPalign.h>
#include <string>

#include "TFile.h"
#include "TTree.h"

namespace Belle2 {

  /**
   * Alignment of TOP
   */
  class TOPAlignerModule : public Module {

  public:
    /**
     * number of alignment parameters
     */
    enum {c_numPar = 7};

    /**
     * Constructor
     */
    TOPAlignerModule();

    /**
     * Destructor
     */
    virtual ~TOPAlignerModule();

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

    double m_minBkgPerBar = 0;    /**< minimal assumed background photons per module */
    double m_scaleN0 = 0;         /**< scale factor for figure-of-merit N0 */
    int m_targetMid = 1;          /**< target module to align. Must be 1 <= Mid <= 16 */
    int m_maxFails = 100;         /**< maximum allowed number of failed iterations */
    int m_countFails = 0;         /**< counter for failed iterations */

    TOP::TOPalign m_align = TOP::TOPalign(m_targetMid);       /**< alignment object (consider just one module at a time) */

    // output file

    std::string m_outFileName = "TopAlignPars.root";     /**< Root output file name containing alignment results */
    TFile* m_file = 0;                 /**< TFile */
    TTree* m_alignTree = 0;            /**< TTree containing alignment parameters */

    int m_ntrk = 0;                      /**< track number of the iterative procedure */
    int m_errorCode = 0;                 /**< error code of the alignment procedure */
    std::vector<float> m_vAlignPars;     /**< alignment parameters, to be saved at each iteration */
    std::vector<float> m_vAlignParsErr;  /**< error on alignment parameters, to be saved at each iteration */

  };

} // Belle2 namespace

