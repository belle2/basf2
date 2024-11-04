/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//DQM
#include <dqm/core/DQMHistAnalysis.h>

#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/dbobjects/CDCChannelMap.h>
#include <cdc/dbobjects/CDCGeometry.h>

#include <TH2Poly.h>


namespace Belle2 {

  /**
   * Make summary of data quality from reconstruction
   */
  class DQMHistAnalysisCDCMonObjModule final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisCDCMonObjModule();

    /**
     * Destructor
     */
    ~DQMHistAnalysisCDCMonObjModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    void beginRun() override final;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    void endRun() override final;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    void terminate() override final;

    /**
     * make bad channel list.
     */
    void makeBadChannelList();


    /**
     * Get mean of ADC histogram excluding 0-th bin.
     */
    float getHistMean(TH1D* h) const;

    /**
     * Get median of ADC histogram excluding 0-th bin.
     */
    float getHistMedian(TH1D* h) const;

    /**
     * Get board/channel from layer/wire.
     */
    std::pair<int, int> getBoardChannel(unsigned short layer, unsigned short wire);

    /**
     * Configure bins of TH2Poly.
     */
    void configureBins(TH2Poly* h);
  protected:
    //TObjects for DQM analysis
    TCanvas* m_cMain = nullptr; /**< main panel */
    TCanvas* m_cADC = nullptr; /**< ADC panel */
    TCanvas* m_cTDC = nullptr; /**< TDC panel */
    TCanvas* m_cHit = nullptr; /**< Hit panel */

    MonitoringObject* m_monObj = nullptr; /**< monitoring object */

    TH2F* m_hADC = nullptr; /**< Summary of ADC histograms with track associated hits*/
    TH2F* m_hTDC = nullptr; /**< Summary of TDC histograms with track associated hits*/
    TH2F* m_hHit = nullptr; /**< Summary of hit histograms */
    TH1D* m_hADCs[300]; /**< ADC histograms with track associated hits for each board (0-299) */
    TH1D* m_hTDCs[300]; /**< TDC histograms with track associated hits for each board (0-299) */
    TH1D* m_hHits[56]; /**< hit histograms for each layer (0-55) */

    TH2Poly* h2p = nullptr; /**< bad wires in xy view */
    TH2F* hBadChannel = nullptr; /**< bad channel map;wire;layer */
    TH2F* hBadChannelBC = nullptr; /**< bad channel map per board/channel;board;channel */

    std::vector<std::pair<int, int>> m_badChannels = {}; /**< bad wires list */
    std::map<WireID, std::pair<int, int>> m_chMap = {}; /**< Channel map retrieved  */
    DBArray<CDCChannelMap>* m_channelMapFromDB = nullptr; /**< Channel map retrieved from DB. */
    float m_senseR[56] = {}; /**< Radius of sense (+field) layer.  */
    float m_fieldR[57] = {}; /**< Radius of field layer.  */
    float m_offset[56] = {}; /**< Offset of sense layer  */
    int m_nSenseWires[56] = {}; /**< number of wires for each layer.  */
    DBObjPtr<CDCGeometry>* m_cdcGeo = nullptr; /**< Geometry of CDC */
  };

} // Belle2 namespace


