/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisSVDGeneral.h
// Description : module for DQM histogram analysis of SVD sensors occupancies
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>
#include <vxd/dataobjects/VxdID.h>
#include <svd/dataobjects/SVDSummaryPlots.h>

#include <TFile.h>
#include <TText.h>
#include <TPaveText.h>
#include <TCanvas.h>
#include <TH2F.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisSVDGeneralModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisSVDGeneralModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisSVDGeneralModule();

    /**
     * Initializer.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     */
    void beginRun() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override final;

  private:

    // parameters
    bool m_printCanvas; /**< if true print the pdf of the canvases */
    bool m_3Samples; /**< if true enable 3 samples histograms analysis */
    double m_occError; /**<error level of the occupancy */
    double m_occWarning; /**< warning level of the occupancy */
    double m_occEmpty; /**<empty level of the occupancy */

    double m_onlineOccError; /**<error level of the onlineOccupancy */
    double m_onlineOccWarning; /**< warning level of the onlineOccupancy */
    double m_onlineOccEmpty; /**<empty level of the occupancy */

    double m_statThreshold; /**< minimal number of events to compare histograms */
    double m_timeThreshold; /**< difference between mean of cluster time for present and reference run */
    float m_refMeanP; /**< mean of the signal time peak from Physics reference run */
    float m_refMeanC; /**< mean of the signal time peak from Cosmic reference run */

    //! Parameters accesible from basf2 scripts
    //  protected:

    /** Reference Histogram Root file name */
    std::string m_refFileName;
    /** The pointer to the reference file */
    TFile* m_refFile = nullptr;

    TCanvas* m_cUnpacker = nullptr; /**<unpacker plot canvas */

    SVDSummaryPlots* m_hOccupancy = nullptr;  /**< occupancy histos */
    SVDSummaryPlots* m_hOnlineOccupancy = nullptr;  /**< online occupancy histos */
    SVDSummaryPlots* m_hOccupancyGroupId0 = nullptr;  /**< occupancy histos for cluster time group id=0*/
    SVDSummaryPlots* m_hOccupancy3Samples = nullptr;  /**<  occupancy histos for 3 samples*/
    SVDSummaryPlots* m_hOnlineOccupancy3Samples = nullptr;  /**< online occupancy histos for 3 sampels*/

    TCanvas* m_cOccupancyU = nullptr; /**< occupancy U histo canvas */
    TCanvas* m_cOccupancyU3Samples = nullptr; /**< occupancy U histo canvas for 3 sampes */
    TCanvas* m_cOccupancyUGroupId0 = nullptr; /**< occupancy U histo canvas  for cluster time group Id = 0*/

    TCanvas* m_cOccupancyV = nullptr; /**< occupancy V histo canvas */
    TCanvas* m_cOccupancyV3Samples = nullptr; /**< occupancy V histo canvas  for 3 samples*/
    TCanvas* m_cOccupancyVGroupId0 = nullptr; /**< occupancy V histo canvas  for cluster time group Id = 0*/

    /** additional plots flag*/
    bool m_additionalPlots = false;

    TH1F m_hOccupancyChartChip; /**< occupancy chart histo */
    TCanvas* m_cOccupancyChartChip = nullptr; /**< occupancy chart histo canvas */

    TCanvas* m_cOnlineOccupancyU = nullptr; /**< online occupancy U histo canvas */
    TCanvas* m_cOnlineOccupancyU3Samples = nullptr; /**< online occupancy U histo canvas for 3 samples*/
    TCanvas* m_cOnlineOccupancyV = nullptr; /**< online Occupancy V histo canvas */
    TCanvas* m_cOnlineOccupancyV3Samples = nullptr; /**< online Occupancy V histo canvas  for 3 samples*/

    int m_sensors = 0; /**< number of sensors to considired*/
    TH1F m_hStripOccupancyU[172]; /**< u-side strip chart occupancy histos*/
    TCanvas** m_cStripOccupancyU = nullptr; /**< u-side strip chart occupancy canvas*/
    TH1F m_hStripOccupancyV[172]; /**< u-side strip chart occupancy histos*/
    TCanvas** m_cStripOccupancyV = nullptr; /**< u-side strip chart occupancy canvas*/

    TH1F m_hClusterOnTrackTime_L456V; /**< time for clusters on Track for L456V histo*/
    TCanvas* m_cClusterOnTrackTime_L456V = nullptr; /**< time for clusters on Track for L456V canvas*/

    TH1F m_hClusterOnTrackTimeL456V3Samples; /**< time for clusters on Track for L456V histo for 3 samples*/
    TCanvas* m_cClusterOnTrackTimeL456V3Samples = nullptr; /**< time for clusters on Track for L456V canvas for 3 sampples */

    Int_t findBinY(Int_t layer, Int_t sensor); /**< find Y bin corresponding to sensor, occupancy plot*/

    TPaveText* m_legProblem = nullptr; /**< OfflineOccupancy plot legend, problem */
    TPaveText* m_legWarning = nullptr; /**< OfflineOccupancy plot legend, warning */
    TPaveText* m_legNormal = nullptr; /**< OfflineOccupancy plot legend, normal */
    TPaveText* m_legEmpty = nullptr; /**< OfflineOccupancy plot legend, empty */
    TPaveText* m_legError = nullptr; /**< OfflineOccupancy plot legend, error*/
    TPaveText* m_legOnProblem = nullptr; /**< onlineOccupancy plot legend, problem */
    TPaveText* m_legOnWarning = nullptr; /**< onlineOccupancy plot legend, warning */
    TPaveText* m_legOnNormal = nullptr; /**< onlineOccupancy plot legend, normal */
    TPaveText* m_legOnEmpty = nullptr; /**< onlineOccupancy plot legend, empty */
    TPaveText* m_legOnError = nullptr; /**< onlineOccupancy plot legend, error*/

    TPaveText* m_legTiProblem = nullptr; /**< cluster time on tracks plot legend, problem */
    TPaveText* m_legTiNormal = nullptr; /**< cluster time on tracks plot legend, normal */
    TPaveText* m_legTiEmpty = nullptr; /**< cluster time on tracks plot legend, empty */

    TPaveText* m_legTi3Problem = nullptr; /**< cluster time on tracks  for 3 samples plot legend, problem */
    TPaveText* m_legTi3Normal = nullptr; /**< cluster time on tracks for 3 samples  plot legend, normal */
    TPaveText* m_legTi3Empty = nullptr; /**< cluster time on tracks  for 3 samples plot legend, empty */

    Double_t m_unpackError = 0; /**< Maximum bin_content/ # events allowed before throwing ERROR*/
    Int_t m_occUstatus = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error*/
    Int_t m_occVstatus = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error*/
    Int_t m_occU3Samples = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error for 3 samples*/
    Int_t m_occV3Samples = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error for 3 sampels*/

    Int_t m_occUGroupId0 = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error for 3 samples*/
    Int_t m_occVGroupId0 = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error for 3 sampels*/

    Int_t m_onlineOccUstatus = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error*/
    Int_t m_onlineOccVstatus = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error*/

    Int_t m_onlineOccU3Samples = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error for 3 sample*/
    Int_t m_onlineOccV3Samples = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error for 3 sampes*/

    TString m_runtype = nullptr; /**< string with runtype: physics or cosmic */

    std::string m_pvPrefix; /**< string prefix for EPICS PVs */

    //! IDs of all SVD Modules to iterate over
    std::vector<VxdID> m_SVDModules;

  };
} // end namespace Belle2

