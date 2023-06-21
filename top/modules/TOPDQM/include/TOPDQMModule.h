/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// TOP headers.
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRecBunch.h>
#include <top/dbobjects/TOPCalCommonT0.h>

// Basf2 headers.
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <mdst/dataobjects/Track.h>

// ROOT headers.
#include <TH1F.h>
#include <TH1D.h>
#include <TH2F.h>
#include <TProfile.h>

// C++ headers.
#include <string>
#include <vector>

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

  private:

    /**
     * Returns slot ID of the module that is hit by the track
     * @param track charged track
     * @return slotID or 0 if track does not hit any module
     */
    int getModuleID(const Track& track) const;

    // module parameters
    std::string m_histogramDirectoryName; /**< histogram directory in ROOT file */
    double m_momentumCut; /**< momentum cut */

    // histograms
    TH1D* m_BoolEvtMonitor = 0; /**< Event desynchronization monitoring */
    TH2F* m_window_vs_slot = 0; /**< Histogram window w.r.t reference vs. slot number */
    TH1F* m_bunchOffset = 0; /**< reconstructed bunch: current offset */
    TH1F* m_time = 0; /**< time distribution of good hits */
    TProfile* m_signalHits = 0; /**< number of hits in the signal time window vs. slot number */
    TProfile* m_backgroundHits = 0; /**< number of hits in the background time window vs. slot number */

    TH1F* m_goodHitsPerEventAll = 0; /**< Number of good hits per event (all slots) */
    TH1F* m_badHitsPerEventAll = 0;  /**< Number of bad hits per event (all slots) */
    TH1F* m_goodTDCAll = 0; /**< TDC distribution of good hits (all slots) */
    TH1F* m_badTDCAll = 0;  /**< TDC distribution of bad hits (all slots) */

    TH1F* m_TOPOccAfterInjLER = 0; /**< Histogram Ndigits after LER injection */
    TH1F* m_TOPOccAfterInjHER = 0; /**< Histogram Ndigits after HER injection */
    TH1F* m_TOPEOccAfterInjLER = 0; /**< Histogram for Nr Entries (=Triggrs) for normalization after LER injection */
    TH1F* m_TOPEOccAfterInjHER = 0; /**< Histogram for Nr Entries (=Triggrs) for normalization after HER injection */

    std::vector<TH2F*> m_window_vs_asic; /**< Histograms window w.r.t reference vs. ASIC */
    std::vector<TH2F*> m_goodHitsXY; /**< Histograms (2D) for good hits in pixels */
    std::vector<TH2F*> m_badHitsXY; /**< Histograms (2D) for bad hits in pixels */
    std::vector<TH2F*> m_goodHitsAsics; /**< Histograms (2D) for good hits in asic channels */
    std::vector<TH2F*> m_badHitsAsics; /**< Histograms (2D) for bad hits in asic channels */
    std::vector<TH1F*> m_goodTDC; /**< Histograms for TDC distribution of good hits */
    std::vector<TH1F*> m_badTDC; /**< Histograms for TDC distribution of bad hits */
    std::vector<TH1F*> m_goodTiming; /**< Histograms for timing distribution of good hits */
    std::vector<TH1F*> m_goodChannelHits; /**< Histograms for good channel hits */
    std::vector<TH1F*> m_badChannelHits; /**< Histograms for bad channel hits */

    // other
    int m_numModules = 0; /**< number of TOP modules */

    // dataobjects
    StoreArray<RawFTSW> m_rawFTSW; /**< Input array for DAQ Status. */
    StoreArray<TOPDigit> m_digits; /**< collection of digits */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */
    StoreArray<Track> m_tracks;    /**< collection of tracks */

    // dbobjects
    DBObjPtr<TOPCalCommonT0> m_commonT0;   /**< common T0 calibration constants */

  };

} // Belle2 namespace

