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
    /**
     * Define the channel id type
     */
    typedef unsigned long long topcaf_channel_id_t;
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
     * @return 0 for good hit
     */
    int flag_hit(const TOPDigit& digit);

  private:
    std::string m_histogramDirectoryName; /**< histogram directory in ROOT file */
    int m_iEvent = -1; /**< Number of events that has been processed */
    int m_ADC_cut_low = -1, m_ADC_cut_high = -1; /**< hit cut for ADC */
    int m_PulseWidth_cut_low = -1, m_PulseWidth_cut_high = -1; /**< hit cut for PulseWidth */
    int m_TDC_particle_low = -999, m_TDC_particle_high = -999;
    int m_TDC_cal_low = -999, m_TDC_cal_high = -999;
    int m_TDC_laser_low = -999, m_TDC_laser_high = -999;

    TH1F* m_all_hits = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_good_hits = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_bad_hits = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_particle_hits = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_laser_hits = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_cal_hits = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_other_hits = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_good_hits_mean = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_bad_hits_mean = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_good_hits_rms = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_bad_hits_rms = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_particle_hits_mean = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_laser_hits_mean = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_cal_hits_mean = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_other_hits_mean = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_particle_hits_rms = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_laser_hits_rms = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_cal_hits_rms = nullptr; /**< Histogram for number of hits within on event */
    TH1F* m_other_hits_rms = nullptr; /**< Histogram for number of hits within on event */
    std::vector<TH1F*> m_slot_all_hits; /**< Histogram for number of hits for one module within on event */
    std::vector<TH1F*> m_slot_good_hits; /**< Histogram for number of hits for one module within on event */
    std::vector<TH1F*> m_slot_bad_hits; /**< Histogram for number of hits for one module within on event */
    std::vector<TH1F*> m_slot_particle_hits; /**< Histogram for number of hits for one module within on event */
    std::vector<TH1F*> m_slot_laser_hits; /**< Histogram for number of hits for one module within on event */
    std::vector<TH1F*> m_slot_cal_hits; /**< Histogram for number of hits for one module within on event */
    std::vector<TH1F*> m_slot_other_hits; /**< Histogram for number of hits for one module within on event */
    std::vector<TH1F*> m_channel_all_hits; /**< Histogram for number of hits for one module within on event */
    std::vector<TH1F*> m_channel_good_hits; /**< Histogram for number of hits for one module within on event */
    std::vector<TH1F*> m_channel_bad_hits; /**< Histogram for number of hits for one module within on event */
    std::vector<TH1F*> m_channel_particle_hits; /**< Histogram for number of hits for one module within on event */
    std::vector<TH1F*> m_hit_quality; /**< hit quality for each module */
    std::vector<TH2F*> m_all_hits_xy;
    std::vector<TH2F*> m_laser_hits_xy;
    std::vector<TH1F*> m_all_ADC;
    std::vector<TH2F*> m_all_ADC_mean;
    std::vector<TH2F*> m_all_ADC_RMS;
    std::vector<TH1F*> m_all_TDC;
    std::vector<TH2F*> m_all_TDC_mean;
    std::vector<TH2F*> m_all_TDC_RMS;
  };

} //namespace Belle2
