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
#include <top/dataobjects/TOPRawWaveform.h>
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
  class TOPDataQualityOnlineModule : public Module {
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
     * Clear the graphs when necessary
     */
    void clear_graph();

    /**
     * Update the graphs when new histograms are available
     */
    void update_graph();

    /**
     * Draw wavefroms for each event (deprecated)
    * @param wave The pointer to the TOPRawWaveform
     */
    void drawWaveforms(TOPRawWaveform* wave);

  private:
    int m_iFrame = 0; /**< Number of frames within one event */
    int m_iEvent = 0; /**< Number of events that has been processed */
    int m_refreshCount = 0; /**< For how many events should the graphs be updated */
    int m_framesPerEvent = 0; /**< Number of frames within one event */
    int m_nhits = 0; /**< Number of hits within one event */

    std::map<int, std::map<int, TMultiGraph*>> m_channels; /**< map: scrod_id --> (asic_id --> TMultiGraph) */
    std::map<int, std::map<int, std::set<int>>> m_channelLabels; /**< map: scrod_id --> (asic_id --> channels within one asic) */
    std::map<int, TCanvas*> m_canvas; /**< map: scrod_id --> TCanvas */
    TCanvas* m_canvas_nhits = NULL; /**< TCanvas for displaying waveform */
    TH1F* m_h_nhits = NULL; /**< Histogram for number of hits within on event */
  };

} //namespace Belle2
