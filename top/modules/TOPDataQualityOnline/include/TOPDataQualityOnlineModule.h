/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Boqun Wang                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <top/dataobjects/TOPDigit.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TMultiGraph.h"
#include <string>
#include <set>
#include <map>
#include <vector>
#include <deque>
#include <utility>
#include <top/modules/TOPDataQualityOnline/TOPDQStat.h>

namespace Belle2 {
  /**
   * Module for online data quality monitoring
   */
  class TOPDataQualityOnlineModule : public HistoModule {
  public:
    /**
     * Constructor
     */
    TOPDataQualityOnlineModule();

    /**
     * Destructor
     */
    ~TOPDataQualityOnlineModule() {};

    /**
     * Defining the histograms
     */
    void defineHisto();

    /**
     * Initialize the module
     */
    void initialize();

    /**
     * Called when entering a new run
     */
    void beginRun();

    /**
     * Event processor
     */
    void event();

    /**
     * End-of-run action
     */
    void endRun();

    /**
     * Termination action
     */
    void terminate();

    /**
     * Flag hit quality
    * @param digit TOPDigit to be flagged
     * @return 0 for good hit
     */
    int flagHit(const TOPDigit& digit);

  private:
    std::string m_histogramDirectoryName; /**< histogram directory in ROOT file */
    int m_iEvent = -1; /**< Number of events that has been processed */
    int m_ADCCutLow = -1, m_ADCCutHigh = -1; /**< hit cut for ADC */
    int m_PulseWidthCutLow = -1, m_PulseWidthCutHigh = -1; /**< hit cut for PulseWidth */
    int m_TDCParticleLow = -999, m_TDCParticleHigh = -999; /**< hit cut for particle TDC */
    int m_TDCCalLow = -999, m_TDCCalHigh = -999; /**< hit cut for calibration TDC */
    int m_TDCLaserLow = -999, m_TDCLaserHigh = -999; /**< hit cut for laser TDC */
    int m_numModules = -1; /**< number of all modules */
    int m_verbose = -1; /**< flag for verbose mode */

    TH1F* m_allHits = nullptr; /**< Histogram for number of accumulated hits */
    TH1F* m_goodHits = nullptr; /**< Histogram for number of accumulated good hits */
    TH1F* m_badHits = nullptr; /**< Histogram for number of accumulated bad hits */
    TH1F* m_particleHits = nullptr; /**< Histogram for number ofaccumulated particle hits */
    TH1F* m_laserHits = nullptr; /**< Histogram for number ofaccumulated laser hits */
    TH1F* m_calHits = nullptr; /**< Histogram for number ofaccumulated calibration hits */
    TH1F* m_otherHits = nullptr; /**< Histogram for number ofaccumulated other hits */
    TH1F* m_goodHitsMean = nullptr; /**< Histogram for mean of number of good hits per event */
    TH1F* m_badHitsMean = nullptr; /**< Histogram for mean of number of bad hits per event */
    TH1F* m_goodHitsRMS = nullptr; /**< Histogram for RMS of number of good hits per event */
    TH1F* m_badHitsRMS = nullptr; /**< Histogram for RMS of number of bad hits per event */
    TH1F* m_particleHitsMean = nullptr; /**< Histogram for mean of number of particle hits per event */
    TH1F* m_laserHitsMean = nullptr; /**< Histogram for mean of number of laser hits per event */
    TH1F* m_calHitsMean = nullptr; /**< Histogram for mean of number of calibration hits per event */
    TH1F* m_otherHitsMean = nullptr; /**< Histogram for mean of number of other hits per event */
    TH1F* m_particleHitsRMS = nullptr; /**< Histogram for RMS of number of particle hits per event */
    TH1F* m_laserHitsRMS = nullptr; /**< Histogram for RMS of number of laser hits per event */
    TH1F* m_calHitsRMS = nullptr; /**< Histogram for RMS of number of calibration hits per event */
    TH1F* m_otherHitsRMS = nullptr; /**< Histogram for RMS of number of other hits per event */

    std::vector<TH1F*> m_slotAllHits; /**< Histograms for number of hits per event per module*/
    std::vector<TH1F*> m_slotGoodHits; /**< Histograms for number of good hits per event per module*/
    std::vector<TH1F*> m_slotBadHits; /**< Histograms for number of bad hits per event per module*/
    std::vector<TH1F*> m_slotParticleHits; /**< Histograms for number of particle hits per event per module*/
    std::vector<TH1F*> m_slotLaserHits; /**< Histograms for number of laser hits per event per module*/
    std::vector<TH1F*> m_slotCalHits; /**< Histograms for number of calibration hits per event per module*/
    std::vector<TH1F*> m_slotOtherHits; /**< Histograms for number of other hits per event per module*/
    std::vector<TH1F*> m_channelAllHits; /**< Histograms for channel hits per module*/
    std::vector<TH1F*> m_channelGoodHits; /**< Histograms for good channel hits per module*/
    std::vector<TH1F*> m_channelBadHits; /**< Histograms for bad channel hits per module*/
    std::vector<TH1F*> m_channelParticleHits; /**< Histograms for particle channel hits per module*/
    std::vector<TH1F*> m_hitQuality; /**< Histograms for hit quality per module*/
    std::vector<TH2F*> m_allHitsXy; /**< Histograms (2D) for channel hits per module*/
    std::vector<TH2F*> m_laserHitsXy; /**< Histograms (2D) for laser channel hits per module*/
    std::vector<TH1F*> m_allAdc; /**< Histograms for ADC distribution per module*/
    std::vector<TH2F*> m_allAdcMean; /**< Histograms (2D) for ADC mean distribution per module*/
    std::vector<TH2F*> m_allAdcRMS; /**< Histograms (2D) for ADC RMS distribution per module*/
    std::vector<TH1F*> m_allTdc; /**< Histograms for TDC distribution per module*/
    std::vector<TH2F*> m_allTdcMean; /**< Histograms (2D) for TDC mean distribution per module*/
    std::vector<TH2F*> m_allTdcRMS; /**< Histograms (2D) for TDC RMS distribution per module*/

    TOPDQStat m_particleHitsCounter; /**< counter for particle hits */
    TOPDQStat m_laserHitsCounter; /**< counter for lawer hits */
    TOPDQStat m_calHitsCounter; /**< counter for calibration hits */
    TOPDQStat m_otherHitsCounter; /**< counter for other hits */
  };

} //namespace Belle2
