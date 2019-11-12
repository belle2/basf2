/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Dan Santel, Boqun Wang                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <vector>
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TProfile2D.h"

// dataobject classes
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRecBunch.h>
#include <mdst/dataobjects/Track.h>

namespace Belle2 {

  /**
   * TOP DQM histogrammer
   */
  class TOPDQMModule : public HistoModule {

  public:

    /**
     * Constructor
     */
    TOPDQMModule();

    /**
     * Destructor
     */
    virtual ~TOPDQMModule();

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    virtual void defineHisto() override;

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

  private:

    // module parameters
    std::string m_histogramDirectoryName; /**< histogram directory in ROOT file */
    double m_momentumCut; /**< momentum cut */
    double m_pValueCut;   /**< pValue cut */
    bool m_usePionID; /**< use pion ID in track selection */
    int m_cutNphot;   /**< Number of photon cut */

    // histograms
    TH1F* m_recoTime = 0;           /**< time distribution */
    TH1F* m_recoTimeBg = 0;         /**< time distribution background */
    TH1F* m_recoTimeMinT0 = 0;      /**< time distribution in respect to the first peak */
    TH1F* m_recoTimeDiff = 0;       /**< residual distribution */
    TH1F* m_recoPull = 0;           /**< pull distribution */
    TH2F* m_recoTimeDiff_Phic = 0;  /**< residuals vs phiCer */
    TProfile* m_recoPull_Phic = 0;  /**< pulls vs phiCer */

    TH1F* m_goodHits = nullptr; /**< Histogram for number of accumulated good hits */
    TH1F* m_badHits = nullptr; /**< Histogram for number of accumulated bad hits */
    TH2F* m_window_vs_slot = 0; /**< Histogram window w.r.t reference vs. slot number */
    TH1F* m_bunchOffset = 0; /**< reconstructed bunch: current offset */
    TH1F* m_time = 0; /**< time distribution of good hits */
    TProfile* m_hitsPerEvent = 0; /**< a profile histogram of good hits per event */

    std::vector<TH2F*> m_window_vs_asic; /**< Histograms window w.r.t reference vs. ASIC */
    std::vector<TH2F*> m_goodHitsXY; /**< Histograms (2D) for good hits in x-y*/
    std::vector<TH2F*> m_badHitsXY; /**< Histograms (2D) for bad hits in x-y*/
    std::vector<TH2F*> m_goodHitsAsics; /**< Histograms (2D) for good hits for asics*/
    std::vector<TH2F*> m_badHitsAsics; /**< Histograms (2D) for bad hits for asics*/
    std::vector<TH1F*> m_goodTdc; /**< Histograms for TDC distribution of good hits for each slot*/
    std::vector<TH1F*> m_badTdc; /**< Histograms for TDC distribution of bad hits for each slot*/
    std::vector<TH1F*> m_goodTDC; /**< Histograms for TDC distribution of good hits*/
    std::vector<TH1F*> m_badTDC; /**< Histograms for TDC distribution of bad hits*/
    std::vector<TH1F*> m_goodTiming; /**< Histograms for timing distribution of good hits*/
    std::vector<TH1F*> m_goodChannelHits; /**< Histograms for good channel hits*/
    std::vector<TH1F*> m_badChannelHits; /**< Histograms for bad channel hits*/
    std::vector<TH1F*> m_goodHitsPerEvent; /**< Histograms for good hits per event for each slot*/
    std::vector<TH1F*> m_badHitsPerEvent; /**< Histograms for bad hits per event for each slot*/
    std::vector<TH1F*> m_goodHitsPerEventAll; /**< Histograms for good hits per event*/
    std::vector<TH1F*> m_badHitsPerEventAll; /**< Histograms for bad hits per event*/
    std::vector<TProfile2D*> m_goodHitsXYTrack; /**< Histograms (2D) for good hits in x-y per module with track*/
    std::vector<TProfile2D*> m_goodHitsXYTrackBkg; /**< Histograms (2D) for good hits in x-y per module with bkg track*/

    // other
    int m_numModules = 0; /**< number of TOP modules */

    // dataobjects
    StoreArray<TOPDigit> m_digits; /**< collection of digits */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */
    StoreArray<Track> m_tracks;    /**< collection of tracks */

  };

} // Belle2 namespace

