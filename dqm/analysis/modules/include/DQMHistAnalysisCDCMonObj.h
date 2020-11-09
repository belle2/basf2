/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

//DQM
#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/dbobjects/CDCChannelMap.h>
#include <cdc/dbobjects/CDCGeometry.h>

#include <vector>

#include <TCanvas.h>
#include <TLine.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH2Poly.h>
#include <TString.h>


namespace Belle2 {

  /**
   * Make summary of data quality from reconstruction
   */
  class DQMHistAnalysisCDCMonObjModule : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisCDCMonObjModule();

    /**
     * Destructor
     */
    virtual ~DQMHistAnalysisCDCMonObjModule();

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

    /**
    * Find canvas by name
    * @param cname Name of the canvas
    * @return The pointer to the canvas, or nullptr if not found.
    */
    TCanvas* find_canvas(TString cname);


    /**
     * make bad channel list.
     */
    void makeBadChannelList();


    /**
     * Get mean of ADC histgram excluding 0-th bin.
     */
    float getHistMean(TH1D* h);

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
    TCanvas* m_cADC = nullptr; /**< main panel */
    TCanvas* m_cTDC = nullptr; /**< bad wire panel */
    TCanvas* m_cHit = nullptr; /**< main panel */

    MonitoringObject* m_monObj = nullptr; /**< monitoring object */

    TH1F* m_hfastTDC = nullptr; /**< Histogram of num. event */
    TH2F* m_hADC = nullptr; /**< Summary of ADC histograms  */
    TH2F* m_hADCTOTCut = nullptr; /**< Summary of ADC histograms with tot cut*/
    TH2F* m_hTDC = nullptr; /**< Summary of TDC histograms */
    TH2F* m_hHit = nullptr; /**< Summary of hit histograms */
    TH1D* m_hADCs[300]; /**< ADC histograms for each board (0-299) */
    TH1D* m_hADCTOTCuts[300]; /**< ADC histograms with tot cut for each board (0-299) */
    TH1D* m_hTDCs[300]; /**< TDC histograms for each board (0-299) */
    TH1D* m_hHits[56]; /**< hit histograms for each layer (0-55) */

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


