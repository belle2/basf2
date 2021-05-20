//+
// File : DQMHistAnalysisSVDGeneral.h
// Description : module for DQM histogram analysis of SVD sensors occupancies
//
// Author : Giulia Casarosa (PI), Gaetano De Marino (PI)
// Date : 20181128
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <vxd/dataobjects/VxdID.h>

#include <TFile.h>
#include <TText.h>
#include <TPaveText.h>
#include <TCanvas.h>
#include <TH2F.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisSVDGeneralModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisSVDGeneralModule();
    virtual ~DQMHistAnalysisSVDGeneralModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    // parameters
    bool m_printCanvas; /**< if true print the pdf of the canvases */
    float m_occError; /**<error level of the occupancy */
    float m_occWarning; /**< warning level of the occupancy */
    float m_occEmpty; /**<empty level of the occupancy */

    float m_onlineOccError; /**<error level of the onlineOccupancy */
    float m_onlineOccWarning; /**< warning level of the onlineOccupancy */
    float m_onlineOccEmpty; /**<empty level of the occupancy */

    int m_statThreshold; /**< minimal number of events to compare histograms */
    float m_timeThreshold; /**< difference between mean of cluster time for present and reference run */
    float m_refMeanP; /**< mean of the signal time peak from Physics reference run */
    float m_refMeanC; /**< mean of the signal time peak from Cosmic reference run */

    //! Parameters accesible from basf2 scripts
    //  protected:


  private:

    /** Reference Histogram Root file name */
    std::string m_refFileName;
    /** The pointer to the reference file */
    TFile* m_refFile = nullptr;

    TCanvas* m_cUnpacker = nullptr; /**<unpacker plot canvas */
    TH2F* m_hOccupancyU = nullptr; /**< occupancy U histo */
    TCanvas* m_cOccupancyU = nullptr; /**< occupancy U histo canvas */
    TH2F* m_hOccupancyV = nullptr; /**< occupancy V histo */
    TCanvas* m_cOccupancyV = nullptr; /**< occupancy V histo canvas */

    TH1F m_hOccupancyChartChip; /**< occupancy chart histo */
    TCanvas* m_cOccupancyChartChip = nullptr; /**< occupancy chart histo canvas */

    TH2F* m_hOnlineOccupancyU = nullptr; /**< online occupancy U histo */
    TCanvas* m_cOnlineOccupancyU = nullptr; /**< online occupancy U histo canvas */
    TH2F* m_hOnlineOccupancyV = nullptr; /**< online Occupancy V histo */
    TCanvas* m_cOnlineOccupancyV = nullptr; /**< online Occupancy V histo canvas */

    const int nSensors = 172; /**< total number of sensors */
    TH1F m_hStripOccupancyU[172]; /**< u-side strip chart occupancy histos*/
    TCanvas** m_cStripOccupancyU = nullptr; /**< u-side strip chart occupancy canvas*/
    TH1F m_hStripOccupancyV[172]; /**< u-side strip chart occupancy histos*/
    TCanvas** m_cStripOccupancyV = nullptr; /**< u-side strip chart occupancy canvas*/

    TH1F m_hClusterOnTrackTime_L456V; /**< time for clusters on Track for L456V histo*/
    TCanvas* m_cClusterOnTrackTime_L456V = nullptr; /**< time for clusters on Track for L456V canvas*/

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
    TText* m_yTitle = nullptr; /**< y axis title text*/

    Double_t m_unpackError = 0; /**< Maximum bin_content/ # events allowed before throwing ERROR*/
    Int_t m_occUstatus = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error*/
    Int_t m_occVstatus = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error*/
    Int_t m_onlineOccUstatus = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error*/
    Int_t m_onlineOccVstatus = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error*/

    TH1* rtype = nullptr; /**< histogram from DQMInfo with runtype */
    TString runtype = nullptr; /**< string with runtype: physics or cosmic */

    //! IDs of all SVD Modules to iterate over
    std::vector<VxdID> m_SVDModules;

  };
} // end namespace Belle2

