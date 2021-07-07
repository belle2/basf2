/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHDQMMODULE_H
#define ARICHDQMMODULE_H

#include <framework/core/HistoModule.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>

#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawFTSW.h>

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
    virtual void initialize() override;

    virtual void defineHisto() override;

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

    bool m_arichEvents; /**< process only events that have extrapolated hit in arich */
    int m_maxHits; /**< exclude events with very large number of hits in arich */
    int m_minHits; /**< exclude events with number of hits lower than this */

    //Histograms to show status by 1/0
    TH1* h_chStat = NULL;/**<Status of each channels*/
    TH1* h_aeroStat = NULL;/**<Status of each aerogel tiles*/

    //Hitograms to show the data quality
    TH1* h_chDigit   = NULL;/**<The number of raw digits in each channel*/
    TH1* h_chipDigit = NULL;/**<The number of raw digits in each ASIC chip*/
    TH1* h_hapdDigit = NULL;/**<The number of raw digits in each HAPD*/
    TH1* h_chHit = NULL;/**<The number of hits in each channel*/
    TH1* h_chipHit = NULL;/**<The number of hits in each ASIC chip*/
    TH1* h_hapdHit = NULL;/**<The number of hits in each HAPD*/
    TH1* h_mergerHit = NULL;/**<The number of hits in each Merger Boards*/
    TH2* h_bitsPerMergerNorm =
      NULL;/*<The number of hits in each bit in each Merger Board normalised to number of HAPDs and sum(bit1, bit2)>*/
    TH2* h_bitsPerHapdMerger = NULL;/*<The number of hits in each bit in each HAPD sorted by Merger Board>*/
    TH1* h_secHapdHit[6] = {};/**<The number of hits in each HAPDs of each sector*/
    TH2* h_hapdHitPerEvent = NULL; /**< number of hits in each HAPD per event */
    TH1* h_aerogelHit = NULL;/**<The number of reconstructed photons in each aerogel tiles*/
    TH1* h_bits = NULL;/**<Timing bits*/
    TH2* h_bitsPerChannel = NULL;/**/
    TH2* h_hitsPerTrack2D = NULL;/**<Sum of 2D hit/track map on each position of track*/
    TH2* h_tracks2D = NULL;/**<2D track distribution of whole ARICH*/
    TH3* h_aerogelHits3D = NULL; /**< 3D histogram of */
    TH3* h_mirrorThetaPhi = NULL; /**< cherenkov theta vs phi for mirror reflected photons (for each mirror plate)*/
    TH2* h_thetaPhi = NULL;  /**< cherenkov theta vs phi for non-mirror-reflected photons*/
    TH1* h_hitsPerEvent = NULL;/**<Ihe number of all hits in each event*/
    TH1* h_theta = NULL;/**<Reconstructed Cherenkov angles*/
    TH1* h_hitsPerTrack = NULL;/**<Average hits/track calculated from h_hits2D and h_track2D*/
    TH1* h_trackPerEvent = NULL;/**<Number of tracks in ARICH per event (with p>0.5 GeV)*/
    TH1* h_flashPerAPD = NULL;/**<Number of flashes in each APD */

    TH1* h_secTheta[6] = {};/**<Detailed view of Cherenkov angle for each sector*/
    TH1* h_secHitsPerTrack[6] = {};/**<Detailed average hits/track for each sector*/

    TH1* h_ARICHOccAfterInjLer = NULL; /**< Histogram Ndigits after LER injection */
    TH1* h_ARICHOccAfterInjHer = NULL; /**< Histogram Ndigits after HER injection */
    TH1* h_ARICHEOccAfterInjLer = NULL; /**< Histogram for Nr Entries (=Triggrs) for normalization after LER injection */
    TH1* h_ARICHEOccAfterInjHer = NULL; /**< Histogram for Nr Entries (=Triggrs) for normalization after HER injection */
    // dataobjects
    StoreArray<RawFTSW> m_rawFTSW; /**< Input array for DAQ Status. */

    //Monitoring parameters

    double m_momUpLim = 0;/**<Upper momentum limit of tracks used in GeV (if set 0, no limit is applied)*/
    double m_momDnLim = 0;/**<Lower momentum limit of tracks used in GeV (if set 0, no limit is applied)*/

  };

} // Belle2 namespace

#endif
