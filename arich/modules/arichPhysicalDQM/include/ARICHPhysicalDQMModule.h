/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Kindo Haruki                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHPYSICALDQMMODULE_H
#define ARICHPYSICALDQMMODULE_H

#include <framework/core/HistoModule.h>

#include <arich/modules/arichPhysicalDQM/newTHs.h>

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
  class ARICHPhysicalDQMModule : public HistoModule {

  public:

    /**
     * Constructor
     */
    ARICHPhysicalDQMModule();

    /**
     * Destructor
     */
    virtual ~ARICHPhysicalDQMModule();

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
    bool m_debug;
    std::string m_outputFileName;

    TFile* results = NULL;

    TH1* h_chStat = NULL;
    TH1* h_aeroStat = NULL;

    TH1* h_chHit = NULL;
    TH1* h_chipHit = NULL;
    TH1* h_hapdHit = NULL;
    TH1* h_mergerHit = NULL;
    TH1* h_gelHit = NULL;
    TH1* h_bits = NULL;
    TH2* h_hits2D = NULL;
    TH2* h_tracks2D = NULL;
    TH2* h_gelHits2D[124] = {};
    TH2* h_gelTracks2D[124] = {};
    TH1* h_mergersHit[72] = {};

    TH1* h_hitsPerEvent = NULL;
    TH1* h_theta = NULL;
    TH1* h_hitsPerTrack = NULL;

    TH1* h_secTheta[6] = {};
    TH1* h_secHitsPerTrack[6] = {};

    //monitoring parameters
    double m_momUpLim = 0;
    double m_momDnLim = 0;

  };

} // Belle2 namespace

#endif
