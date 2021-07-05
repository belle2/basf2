/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

// from basf2
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <top/dataobjects/TOPRawWaveform.h>

// stl
#include <string>
#include <set>
#include <map>

// ROOT
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TMultiGraph.h"
#include "TDirectory.h"

namespace Belle2 {
  /**
   * Plots and histograms of waveforms and feature extracted parameters
   */
  class TOPWaveformQualityPlotterModule : public HistoModule {
    typedef unsigned long long top_channel_id_t; /**< topcaf channel ID (deprecated?) */
  public:
    /**
     * Constructor
     */
    TOPWaveformQualityPlotterModule();

    /**
     * Books the empty histograms
     */
    void defineHisto() override;

    /**
     * Module initialization, calls defineHisto and gets waveform
     */
    void initialize() override;

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
     * Draws the full waveforms onto the TProfiles
     * @param rawwave the raw waveform
     */
    void drawWaveforms(const TOPRawWaveform& rawwave);

    /**
     * Fills the debugging 1D histograms and hitmaps
     * @param rawwave the raw waveform
     */
    void basicDebuggingPlots(const TOPRawWaveform& rawwave);

  private:
    int m_iEvent = -1;             /**< keeps track of iterations within run */
    TH1F* m_samples = 0;           /**< plot of ADC sample values */
    std::map<int, TH2F*> m_hitmap; /**< hitmaps for each SCROD */
    TH1F* m_scrod_id = 0;          /**< plot of SCROD IDs for debugging */
    TH1F* m_asic = 0;              /**< plot of ASIC number for debugging */
    TH1F* m_carrier = 0;           /**< plot of carrier IDs for debugging */
    TH1F* m_asic_ch = 0;           /**< plot of ASIC channel ID debugging */
    TH1F* m_errorFlag = 0;         /**< plot of error flag (not impemented) */
    TH1F* m_asic_win = 0;          /**< plot of ASIC storage window */
    TH1F* m_entries = 0;           /**< plot of waveform size */
    TH1F* m_moduleID = 0;          /**< plot of module ID */
    TH1F* m_pixelID = 0;           /**< plot of pixel ID for debugging */

    StoreArray<TOPRawWaveform> m_waveform; /**< the raw waveforms */
    std::map<top_channel_id_t, TH1F*> m_channelNoiseMap; /**< histogram of the values after correction */
    std::map<top_channel_id_t, int> m_channelEventMap; /**< to find in which chunk a given channel is */

    std::map<int, std::map<std::string, TMultiGraph*>> m_channels; /**< per board stack, per asic */
    std::map<int, std::map<std::string, std::set<int>>> m_channelLabels; /**< per board stack, per asic */

    std::map<int, std::map<int, std::map<int, std::map<int, TProfile*>>>> m_waveformHists; /**< scrod, carrier, asic, channel */

    bool m_DRAWWAVES = true; /**< option to draw waveforms */
    bool m_DEBUGGING = true; /**< option to fill debug histograms */
    bool m_NOISE = false;    /**< option to draw noisemaps */

    std::string m_histogramDirectoryName; /**< the name of the directory inside the output file */
    TDirectory* m_directory = 0;          /**< the directory itself */

  };

}
