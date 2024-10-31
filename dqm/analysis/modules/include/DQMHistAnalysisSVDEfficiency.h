/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisSVDEfficiency.h
// Description : module for DQM histogram analysis of SVD sensors efficiencies
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysisSVD.h>
#include <svd/dataobjects/SVDSummaryPlots.h>

#include <TFile.h>
#include <TPaveText.h>
#include <TCanvas.h>
#include <TText.h>
#include <TLine.h>
#include <TArrow.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisSVDEfficiencyModule final : public DQMHistAnalysisSVDModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisSVDEfficiencyModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisSVDEfficiencyModule();

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

    //parameters
    double m_effError; /**<error level of the efficiency */
    double m_effWarning; /**< warning level of the efficiency */
    double m_statThreshold; /**<minimal number of tracks per sensor to set green or red frame */
    bool m_3Samples; /**< if true enable 3 samples histograms analysis */

    //! Data members
    TCanvas* m_cEfficiencyU = nullptr; /**< efficiency U plot canvas */
    TCanvas* m_cEfficiencyV = nullptr; /**< efficiency V plot canvas */
    SVDSummaryPlots* m_hEfficiency = nullptr; /**< efficiency histo */
    TCanvas* m_cEfficiencyErrU = nullptr; /**<efficiency U error plot canvas */
    TCanvas* m_cEfficiencyErrV = nullptr; /**<efficiency V error plot canvas */
    SVDSummaryPlots* m_hEfficiencyErr = nullptr; /**< efficiency error histo */

    TCanvas* m_cEfficiencyU3Samples = nullptr; /**< efficiency U plot canvas  for 3 samples */
    TCanvas* m_cEfficiencyV3Samples = nullptr; /**< efficiency V plot canvas  for 3 samples */
    SVDSummaryPlots* m_hEfficiency3Samples = nullptr; /**< efficiency histo for 3 samples */
    TCanvas* m_cEfficiencyErrU3Samples = nullptr; /**<efficiency U error plot canvas for 3 samples*/
    TCanvas* m_cEfficiencyErrV3Samples = nullptr; /**<efficiency V error plot canvas for 3 samples*/
    SVDSummaryPlots* m_hEfficiencyErr3Samples = nullptr; /**< efficiency error histo for 3 samples*/

    TCanvas* m_cEfficiencyRPhiViewU = nullptr; /**< efficiency U plot canvas */
    TCanvas* m_cEfficiencyRPhiViewV = nullptr; /**< efficiency V plot canvas */
    TCanvas* m_cEfficiencyErrRPhiViewU = nullptr; /**<efficiency U error plot canvas */
    TCanvas* m_cEfficiencyErrRPhiViewV = nullptr; /**<efficiency V error plot canvas */

    TCanvas* m_cEfficiencyRPhiViewU3Samples = nullptr; /**< efficiency U plot canvas  for 3 samples */
    TCanvas* m_cEfficiencyRPhiViewV3Samples = nullptr; /**< efficiency V plot canvas  for 3 samples */
    TCanvas* m_cEfficiencyErrRPhiViewU3Samples = nullptr; /**<efficiency U error plot canvas for 3 samples*/
    TCanvas* m_cEfficiencyErrRPhiViewV3Samples = nullptr; /**<efficiency V error plot canvas for 3 samples*/

    Int_t findBinY(Int_t layer, Int_t sensor); /**< find Y bin corresponding to sensor, efficiency plot*/
    void updateCanvases(SVDSummaryPlots* histo, TCanvas* canvas, TCanvas* canvasRPhi, svdStatus occupancyStatus, float minEff,
                        int side); /**< update canvases */
    void updateErrCanvases(SVDSummaryPlots* histo, TCanvas* canvas, TCanvas* canvasRPhi, int side); /**< update error canvases */

    int m_efficiencyMin = 0; /**< Minimum of the efficiency histogram */
    int m_efficiencyMax = -1111; /**< Maximum of the efficiency histogram. -1111 adjust the maximum depennding on the content */
    bool m_setEfficiencyRange = false; /**< set the range of the efficiency histogram */

    svdStatus m_effUstatus; /**< number representing the status of the efficiency U side */
    svdStatus m_effVstatus;/**< number representing the status of the efficiency V side */

    void setEffStatus(float den, float eff, bool sideU = false); /**< set efficiency status */

    //! IDs of all SVD Modules to iterate over
    std::vector<VxdID> m_SVDModules;

    std::string m_pvPrefix; /**< string prefix for EPICS PVs */
  };
} // end namespace Belle2

