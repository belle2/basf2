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
    int flag_hit(const TOPDigit& digit);

  private:
    std::string m_histogramDirectoryName; /**< histogram directory in ROOT file */
    int m_iEvent = -1; /**< Number of events that has been processed */
    int m_ADC_cut_low = -1, m_ADC_cut_high = -1; /**< hit cut for ADC */
    int m_PulseWidth_cut_low = -1, m_PulseWidth_cut_high = -1; /**< hit cut for PulseWidth */
    int m_TDC_particle_low = -999, m_TDC_particle_high = -999; /**< hit cut for particle TDC */
    int m_TDC_cal_low = -999, m_TDC_cal_high = -999; /**< hit cut for calibration TDC */
    int m_TDC_laser_low = -999, m_TDC_laser_high = -999; /**< hit cut for laser TDC */

    TH1F* m_all_hits = nullptr; /**< Histogram for number of accumulated hits */
    TH1F* m_good_hits = nullptr; /**< Histogram for number of accumulated good hits */
    TH1F* m_bad_hits = nullptr; /**< Histogram for number of accumulated bad hits */
    TH1F* m_particle_hits = nullptr; /**< Histogram for number ofaccumulated particle hits */
    TH1F* m_laser_hits = nullptr; /**< Histogram for number ofaccumulated laser hits */
    TH1F* m_cal_hits = nullptr; /**< Histogram for number ofaccumulated calibration hits */
    TH1F* m_other_hits = nullptr; /**< Histogram for number ofaccumulated other hits */
    TH1F* m_good_hits_mean = nullptr; /**< Histogram for mean of number of good hits per event */
    TH1F* m_bad_hits_mean = nullptr; /**< Histogram for mean of number of bad hits per event */
    TH1F* m_good_hits_rms = nullptr; /**< Histogram for RMS of number of good hits per event */
    TH1F* m_bad_hits_rms = nullptr; /**< Histogram for RMS of number of bad hits per event */
    TH1F* m_particle_hits_mean = nullptr; /**< Histogram for mean of number of particle hits per event */
    TH1F* m_laser_hits_mean = nullptr; /**< Histogram for mean of number of laser hits per event */
    TH1F* m_cal_hits_mean = nullptr; /**< Histogram for mean of number of calibration hits per event */
    TH1F* m_other_hits_mean = nullptr; /**< Histogram for mean of number of other hits per event */
    TH1F* m_particle_hits_rms = nullptr; /**< Histogram for RMS of number of particle hits per event */
    TH1F* m_laser_hits_rms = nullptr; /**< Histogram for RMS of number of laser hits per event */
    TH1F* m_cal_hits_rms = nullptr; /**< Histogram for RMS of number of calibration hits per event */
    TH1F* m_other_hits_rms = nullptr; /**< Histogram for RMS of number of other hits per event */

    std::vector<TH1F*> m_slot_all_hits; /**< Histograms for number of hits per event per module*/
    std::vector<TH1F*> m_slot_good_hits; /**< Histograms for number of good hits per event per module*/
    std::vector<TH1F*> m_slot_bad_hits; /**< Histograms for number of bad hits per event per module*/
    std::vector<TH1F*> m_slot_particle_hits; /**< Histograms for number of particle hits per event per module*/
    std::vector<TH1F*> m_slot_laser_hits; /**< Histograms for number of laser hits per event per module*/
    std::vector<TH1F*> m_slot_cal_hits; /**< Histograms for number of calibration hits per event per module*/
    std::vector<TH1F*> m_slot_other_hits; /**< Histograms for number of other hits per event per module*/
    std::vector<TH1F*> m_channel_all_hits; /**< Histograms for channel hits per module*/
    std::vector<TH1F*> m_channel_good_hits; /**< Histograms for good channel hits per module*/
    std::vector<TH1F*> m_channel_bad_hits; /**< Histograms for bad channel hits per module*/
    std::vector<TH1F*> m_channel_particle_hits; /**< Histograms for particle channel hits per module*/
    std::vector<TH1F*> m_hit_quality; /**< Histograms for hit quality per module*/
    std::vector<TH2F*> m_all_hits_xy; /**< Histograms (2D) for channel hits per module*/
    std::vector<TH2F*> m_laser_hits_xy; /**< Histograms (2D) for laser channel hits per module*/
    std::vector<TH1F*> m_all_ADC; /**< Histograms for ADC distribution per module*/
    std::vector<TH2F*> m_all_ADC_mean; /**< Histograms (2D) for ADC mean distribution per module*/
    std::vector<TH2F*> m_all_ADC_RMS; /**< Histograms (2D) for ADC RMS distribution per module*/
    std::vector<TH1F*> m_all_TDC; /**< Histograms for TDC distribution per module*/
    std::vector<TH2F*> m_all_TDC_mean; /**< Histograms (2D) for TDC mean distribution per module*/
    std::vector<TH2F*> m_all_TDC_RMS; /**< Histograms (2D) for TDC RMS distribution per module*/
  };

} //namespace Belle2
