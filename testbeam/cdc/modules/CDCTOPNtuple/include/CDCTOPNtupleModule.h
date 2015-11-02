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
#include <top/modules/TOPNtuple/NtupleStructure.h>
#include <top/geometry/TOPGeometryPar.h>
#include "TFile.h"

namespace Belle2 {

  /**
   * Module to write out ntuples for the analysis of CDC-TOP cosmic tests.
   * Used for the validation.
   */
  class CDCTOPNtupleModule : public Module {

  public:

    /**
     * Constructor
     */
    CDCTOPNtupleModule();

    /**
     * Destructor
     */
    virtual ~CDCTOPNtupleModule();

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

    TFile* m_file = 0;      /**< TFile */
    TTree* m_treeTop = 0;   /**< TTree with TOPTree structure */
    TOP::TOPTree m_top;     /**< ntuple structure */
    float m_dedx = 0;       /**< additional var: dedx */
    float m_dedxErr = 0;    /**< additional var: error on dedx */
    float m_theta = 0;      /**< additional var: angle btw. upper and lower track arm */

    TTree* m_treePulls = 0;   /**< TTree for pulls */
    int m_pixelID = 0;   /**< software channel ID (1-based) */
    float m_t = 0;       /**< photon time */
    float m_t0 = 0;      /**< PDF mean time (signal) or minimal PDF mean (background) */
    float m_sigma = 0;   /**< PDF sigma (signal) or 0 (background) */
    float m_phiCer = 0;  /**< azimuthal Cerenkov angle */
    float m_wt = 0;      /**< weight */

    TOP::TOPGeometryPar* m_topgp = TOP::TOPGeometryPar::Instance();  /**< geometry */

  };

} // Belle2 namespace

