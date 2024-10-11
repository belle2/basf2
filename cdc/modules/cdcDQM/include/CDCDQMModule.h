/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRawHit.h>

#include <mdst/dataobjects/TRGSummary.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/HitPatternCDC.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <rawdata/dataobjects/RawCDC.h>

class TDirectory;
class TH1F;
class TH2F;
class TH2Poly;

namespace Belle2 {

  /**
   * Make summary of data quality from reconstruction
   */
  class CDCDQMModule : public HistoModule {

  public:

    /**
     * Constructor
     */
    CDCDQMModule();

    /**
     * Destructor
     */
    virtual ~CDCDQMModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    void initialize() override;

    /**
     * Histogram definitions.
     *
     */
    void defineHisto() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    void beginRun() override;

    /**
     * Event processor.
     */
    void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    void terminate() override;

    /**
     * Convenient function to create a TH2Poly based on CDC geometry
     */
    TH2Poly* createTH2Poly(const TString& name, const TString& title, TDirectory* dir) ;

  protected:

    StoreArray<CDCHit> m_cdcHits;                 /**< CDC hits. */
    StoreArray<CDCRawHit> m_cdcRawHits;           /**< CDC raw hits. */
    StoreArray<RawCDC> m_rawCDCs;                 /**< Input array for CDC Raw */
    StoreObjPtr<TRGSummary> m_trgSummary;         /**< Trigger summary. */
    StoreArray<Track> m_Tracks;                   /**< Tracks. */
    StoreArray<RecoTrack> m_RecoTracks;           /**< RecoTracks. */
    std::string m_recoTrackArrayName = "" ;       /**< Belle2::RecoTrack StoreArray name. */
    StoreObjPtr<SoftwareTriggerResult> m_TrgResult; /**< Store array for Trigger selection */

    int m_minHits;                   /**< Minimum hits for processing. */
    Long64_t m_nEvents = 0;          /**< Number of events processed */
    TH1F* m_hNEvents = nullptr;      /**< Histogram of num. events */
    TH1F* m_hOcc = nullptr;          /**< Histogram of occupancy */
    TH2F* m_hADC  = nullptr;         /**< Histogram of ADC with track associated hits for all boards (0-299) */
    TH2F* m_hTDC = nullptr;          /**< Histogram of TDC with track associated hits for all boards (0-299) */
    TH2F* m_hHit  = nullptr;         /**< Histogram of Hits for all layers (0-55) */
    TH2F* m_hBit = nullptr;          /**< Histogram of online databit removed */
    TH1F* m_hPhi = nullptr;          /**< Histogram of cdc phi of IP tracks*/
    TH2F* m_hPhiIndex = nullptr;     /**< Histogram of cdc phi of different IP + skims tracks */
    TH2F* m_hPhiEff = nullptr;       /**< Histogram of cdc phi of tracking eff */
    TH2F* m_hPhiHit  = nullptr;      /**< Histogram of track associated hits in phi vs layer  */
    TH2Poly* m_hObservedExtPos = nullptr; /**< Histogram of observed CDC hits in layer */
    TH2Poly* m_hExpectedExtPos = nullptr; /**< Histogram of expected CDC hits in layer */

  };

} // Belle2 namespace
