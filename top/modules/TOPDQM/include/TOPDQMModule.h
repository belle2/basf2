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

// dataobject classes
#include <framework/datastore/StoreArray.h>
#include <top/dataobjects/TOPDigit.h>
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
    virtual void defineHisto();

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

    // module parameters
    std::string m_histogramDirectoryName; /**< histogram directory in ROOT file */
    double m_momentumCut; /**< momentum cut */
    double m_pValueCut;   /**< pValue cut */
    bool m_usePionID; /**< use pion ID in track selection */
    int m_ADCCutLow = 0; /**< lower cut for ADC */
    int m_ADCCutHigh = 0; /**< upper cut for ADC */
    int m_PulseWidthCutLow = 0; /**< lower cut for PulseWidth */
    int m_PulseWidthCutHigh = 0; /**< upper cut for PulseWidth */

    // histograms
    TH1F* m_recoTime = 0;           /**< time distribution */
    TH1F* m_recoTimeBg = 0;         /**< time distribution background */
    TH1F* m_recoTimeMinT0 = 0;      /**< time distribution in respect to the first peak */
    TH1F* m_recoTimeDiff = 0;       /**< residual distribution */
    TH1F* m_recoPull = 0;           /**< pull distribution */
    TH2F* m_recoTimeDiff_Phic = 0;  /**< residuals vs phiCer */
    TProfile* m_recoPull_Phic = 0;  /**< pulls vs phiCer */

    TH1F* m_particleHits = nullptr; /**< Histogram for number ofaccumulated particle hits */
    TH1F* m_otherHits = nullptr; /**< Histogram for number ofaccumulated other hits */
    std::vector<TH2F*> m_allHitsXY; /**< Histograms (2D) for channel hits per module*/
    std::vector<TH1F*> m_allTdc; /**< Histograms for TDC distribution per module*/
    std::vector<TH1F*> m_particleChannelHits; /**< Histograms for particle channel hits per module*/
    std::vector<TH1F*> m_particleHitsPerEvent; /**< Histograms for particle hits per event per module*/
    std::vector<TH1F*> m_otherHitsPerEvent; /**< Histograms for other hits per event per module*/

    // other
    int m_numModules = 0; /**< number of TOP modules */

    // dataobjects
    StoreArray<TOPDigit> m_digits; /**< collection of digits */
    StoreArray<Track> m_tracks;    /**< collection of tracks */

  };

} // Belle2 namespace

