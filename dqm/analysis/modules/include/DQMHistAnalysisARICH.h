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

//#pragma once

#include <framework/core/Module.h>

//DQM
#include <dqm/analysis/modules/DQMHistAnalysis.h>

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

    //Histograms to show status by 1/0
    TH1* m_h_chStat = NULL;/**<Status of each channels*/
    TH1* m_h_aeroStat = NULL;/**<Status of each aerogel tiles*/

    //Hitograms from DQM module
    TH1* m_h_chDigit   = NULL;/**<The number of raw digits in each channel*/
    TH1* m_h_chipDigit = NULL;/**<The number of raw digits in each ASIC chip*/
    TH1* m_h_hapdDigit = NULL;/**<The number of raw digits in each HAPD*/
    TH1* m_h_chHit = NULL;/**<The number of hits in each channel*/
    TH1* m_h_chipHit = NULL;/**<The number of hits in each ASIC chip*/
    TH1* m_h_hapdHit = NULL;/**<The number of hits in each HAPD*/
    TH1* m_h_mergerHit = NULL;/**<The number of hits in each Merger Boards*/
    TH1* m_h_secHapdHit[6] = {};/**<The number of hits in each HAPDs of each sector*/
    TH2* m_h_hapdHitPerEvent = NULL; /**< number of hits in each HAPD per event */
    TH1* m_h_aerogelHit = NULL;/**<The number of reconstructed photons in each aerogel tiles*/
    TH1* m_h_bits = NULL;/**<Timing bits*/
    TH2* m_h_hitsPerTrack2D = NULL;/**<Sum of 2D hit/track map on each position of track*/
    TH2* m_h_tracks2D = NULL;/**<2D track distribution of whole ARICH*/
    TH3* m_h_aerogelHits3D = NULL; /**< 3D histogram of */
    TH3* m_h_mirrorThetaPhi = NULL; /**< cherenkov theta vs phi for mirror reflected photons (for each mirror plate)*/
    TH2* m_h_thetaPhi = NULL;  /**< cherenkov theta vs phi for non-mirror-reflected photons*/
    TH1* m_h_hitsPerEvent = NULL;/**<Ihe number of all hits in each event*/
    TH1* m_h_theta = NULL;/**<Reconstructed Cherenkov angles*/
    TH1* m_h_hitsPerTrack = NULL;/**<Average hits/track calculated from h_hits2D and h_track2D*/

    TH1* m_h_secTheta[6] = {};/**<Detailed view of Cherenkov angle for each sector*/
    TH1* m_h_secHitsPerTrack[6] = {};/**<Detailed average hits/track for each sector*/

    TLine* m_LineForMB[5] = {}; /**Lines to divide the sectors on mergerHit histogram*/
    TCanvas* m_c_mergerHit = NULL;

  };

} // Belle2 namespace

#endif
