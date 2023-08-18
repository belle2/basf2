/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>

#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>
#include <rawdata/dataobjects/RawFTSW.h>

#include <arich/dbobjects/ARICHGeometryConfig.h>
#include <arich/dbobjects/ARICHMergerMapping.h>

namespace Belle2 {
  class ARICHHit;
  class ARICHDigit;
  class ARICHTrack;
  class ARICHLikelihood;

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

    /**
     * Definition of the histograms.
     */
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

  protected:

    /** Debug. */
    bool m_debug;

    /** Process only events that have extrapolated hit in arich. */
    bool m_arichEvents;

    /** Exclude events with very large number of hits in arich. */
    int m_maxHits;

    /** Exclude events with number of hits lower than this. */
    int m_minHits;

    /* Histograms to show status by 1/0. */

    /** Status of each channel. */
    TH1* h_chStat = NULL;
    /** Status of each aerogel tile. */
    TH1* h_aeroStat = NULL;

    /* Histograms to show the data quality. */

    /** The number of raw digits in each channel. */
    TH1* h_chDigit   = NULL;

    /** The number of raw digits in each ASIC chip. */
    TH1* h_chipDigit = NULL;

    /** The number of raw digits in each HAPD. */
    TH1* h_hapdDigit = NULL;

    /** The number of hits in each channel. */
    TH1* h_chHit = NULL;

    /** The number of hits in each ASIC chip. */
    TH1* h_chipHit = NULL;

    /** The number of hits in each HAPD. */
    TH1* h_hapdHit = NULL;

    /** The number of hits in each Merger Boards. */
    TH1* h_mergerHit = NULL;

    /**
     * The number of hits in each bit in each Merger Board normalised
     * to number of HAPDs and sum(bit1, bit2).
     */
    TH2* h_bitsPerMergerNorm = NULL;

    /** The number of hits in each bit in each HAPD sorted by Merger Board. */
    TH2* h_bitsPerHapdMerger = NULL;

    /** The number of hits in each HAPDs of each sector. */
    TH1* h_secHapdHit[6] = {};

    /** Number of hits in each HAPD per event. */
    TH2* h_hapdHitPerEvent = NULL;

    /** The number of reconstructed photons in each aerogel tile. */
    TH1* h_aerogelHit = NULL;

    /** Timing bits. */
    TH1* h_bits = NULL;

    /** Number of bits per channel. */
    TH2* h_bitsPerChannel = NULL;

    /** Sum of 2D hit/track map on each position of track. */
    TH2* h_hitsPerTrack2D = NULL;

    /** 2D track distribution of whole ARICH. */
    TH2* h_tracks2D = NULL;

    /** 3D histogram of hits. */
    TH3* h_aerogelHits3D = NULL;

    /**
     * Cherenkov theta vs phi for mirror-reflected photons
     * (for each mirror plate).
     */
    TH3* h_mirrorThetaPhi = NULL;

    /** Cherenkov theta vs phi for non-mirror-reflected photons. */
    TH2* h_thetaPhi = NULL;

    /** The number of all hits in each event. */
    TH1* h_hitsPerEvent = NULL;

    /** Reconstructed Cherenkov angles. */
    TH1* h_theta = NULL;

    /** Average hits/track calculated from h_hits2D and h_track2D. */
    TH1* h_hitsPerTrack = NULL;

    /** Number of tracks in ARICH per event (with p>0.5 GeV). */
    TH1* h_trackPerEvent = NULL;

    /** Number of flashes in each APD. */
    TH1* h_flashPerAPD = NULL;

    /** Detailed view of Cherenkov angle for each sector. */
    TH1* h_secTheta[6] = {};

    /** Detailed average hits/track for each sector. */
    TH1* h_secHitsPerTrack[6] = {};

    /** Histogram Ndigits after LER injection. */
    TH1* h_ARICHOccAfterInjLer = NULL;

    /** Histogram Ndigits after HER injection. */
    TH1* h_ARICHOccAfterInjHer = NULL;

    /**
     * Histogram for Nr Entries (=Triggrs) for normalization
     * after LER injection.
     */
    TH1* h_ARICHEOccAfterInjLer = NULL;

    /**
     * Histogram for Nr Entries (=Triggrs) for normalization
     * after HER injection.
      */
    TH1* h_ARICHEOccAfterInjHer = NULL;

    /* Monitoring parameters. */

    /**
     * Upper momentum limit of tracks used in GeV
     * (if set 0, no limit is applied).
     */
    double m_momUpLim = 0;

    /**
     * Lower momentum limit of tracks used in GeV
     * (if set 0, no limit is applied).
     */
    double m_momDnLim = 0;

    /* Dataobjects. */

    /** Input array for DAQ Status. */
    StoreArray<RawFTSW> m_rawFTSW;

    /** ARICHDigits StoreArray. */
    StoreArray<ARICHDigit> m_arichDigits;

    /** ARICHHits StoreArray. */
    StoreArray<ARICHHit> m_arichHits;

    /** ARICHTracks StoreArray. */
    StoreArray<ARICHTrack> m_arichTracks;

    /** ARICHLikelihoods StoreArray. */
    StoreArray<ARICHLikelihood> m_arichLikelihoods;

    /** ARICH Geometry configuration payload. */
    DBObjPtr<ARICHGeometryConfig> m_arichGeoConfig;

    /** ARICH merger mapping payload. */
    DBObjPtr<ARICHMergerMapping> m_arichMergerMap;

  };

} // Belle2 namespace
