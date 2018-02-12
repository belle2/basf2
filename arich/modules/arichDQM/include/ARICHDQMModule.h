/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kindo Haruki                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHDQMMODULE_H
#define ARICHDQMMODULE_H

#include <framework/core/HistoModule.h>

//Functions to make histograms
#include <arich/modules/arichDQM/newTHs.h>

//ARICH dataobjects
#include <arich/dataobjects/ARICHHit.h>
#include <arich/dataobjects/ARICHTrack.h>
#include <arich/dataobjects/ARICHPhoton.h>
#include <arich/dataobjects/ARICHLikelihood.h>

#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TVector2.h>
#include <TText.h>
#include <TFile.h>

#include <vector>
#include <string>
#include <map>

namespace Belle2 {

  /**
   * Make summary of data quality from reconstruction
   */
  class ARICHDQMModule : public HistoModule {

  public:

    /**
     * Constructor
     */
    ARICHDQMModule();

    /**
     * Destructor
     */
    virtual ~ARICHDQMModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    virtual void defineHisto();

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


  protected:
    bool m_debug;/**<debug*/

    //Histograms to show status by 1/0
    TH1* h_chStat = NULL;/**<Status of each channels*/
    TH1* h_aeroStat = NULL;/**<Status of each aerogel tiles*/

    //Hitograms to show the data quality
    TH1* h_chHit = NULL;/**<The number of hits in each channels*/
    TH1* h_chipHit = NULL;/**<The number of hits in each ASIC chips*/
    TH1* h_hapdHit = NULL;/**<The number of hits in each HAPDs*/
    TH1* h_mergerHit = NULL;/**<The number of hits in each Merger Boards*/
    TH1* h_gelHit = NULL;/**<The number of reconstructed photons in each aerogel tiles*/
    TH1* h_bits = NULL;/**<Timing bits*/
    TH2* h_hits2D = NULL;/**<2D hit map of whale ARICH*/
    TH2* h_tracks2D = NULL;/**<2D track distribution of whole ARICH*/
    TH2* h_gelHits2D[124] = {};/**<2D hit maps of each aerogel tiles*/
    TH2* h_gelTracks2D[124] = {};/**<2D track distributions of each aerogel tiles*/

    TH1* h_hitsPerEvent = NULL;/**<Ihe number of all hits in each event*/
    TH1* h_theta = NULL;/**<Reconstructed Cherenkov angles*/
    TH1* h_hitsPerTrack = NULL;/**<Average hits/track calculated from h_hits2D and h_track2D*/

    TH1* h_secTheta[6] = {};/**<Detailed view of Cherenkov angle for each sector*/
    TH1* h_secHitsPerTrack[6] = {};/**<Detailed average hits/track for each sector*/

    //Monitoring parameters

    double m_momUpLim = 0;/**<Upper momentum limit of tracks used in GeV (if set 0, no limit is applied)*/
    double m_momDnLim = 0;/**<Lower momentum limit of tracks used in GeV (if set 0, no limit is applied)*/

  };

} // Belle2 namespace

#endif
