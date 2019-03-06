/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kindo Haruki                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHDQMANALYSISMODULE_H
#define ARICHDQMANALYSISMODULE_H

#pragma once

#include <framework/core/Module.h>

//DQM
#include <dqm/analysis/modules/DQMHistAnalysis.h>

//ARICH
#include <arich/utility/ARICHChannelHist.h>

#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TVector2.h>
#include <TText.h>
#include <TLine.h>
#include <TFile.h>

#include <chrono>
#include <vector>
#include <string>
#include <map>

namespace Belle2 {

  /**
   * Make summary of data quality from reconstruction
   */
  class DQMHistAnalysisARICHModule : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisARICHModule();

    /**
     * Destructor
     */
    virtual ~DQMHistAnalysisARICHModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

  protected:
    bool m_debug;/**<debug*/

    //TObjects for DQM analysis
    TLine* m_LineForMB[5] = {}; /**<Lines to divide the sectors on mergerHit histogram*/
    TCanvas* m_c_mergerHit = NULL; /**<Canvas for modified mergerHit histogram*/

    Belle2::ARICHChannelHist* channelHist = NULL; /**<ARICH TObject to draw hit map for each channel*/
    TCanvas* m_c_channelHist = NULL; /**<Canvas for 2D hit map of channels*/

  };

} // Belle2 namespace

#endif
