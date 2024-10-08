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

#include <dqm/core/DQMHistAnalysis.h>
#include <svd/dataobjects/SVDSummaryPlots.h>

#include <TFile.h>
#include <TPaveText.h>
#include <TCanvas.h>
#include <TText.h>
#include <TLine.h>
#include <TArrow.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisSVDEfficiencyModule final : public DQMHistAnalysisModule {

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

    /** Reference Histogram Root file name */
    std::string m_refFileName;
    /** The pointer to the reference file */
    TFile* m_refFile = nullptr;

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
    std::tuple<std::vector<TText*>, std::vector<TText*>> textModuleNumbers(); /**< create vectors of TText to write on the canvas */
    void drawText(); /**< draw text on the RPhi view */
    void setStatusOfCanvas(int status, TCanvas* canvas, bool plotLeg); /**< set status of Canvas */

    std::vector<TText*> m_laddersText; /**< list of ladders to write on the canvas */
    std::vector<TText*> m_sensorsText; /**< list of sensors to write on the cancas */

    TLine* m_lx = nullptr; /**< y-axis */
    TLine* m_ly = nullptr; /**< x-axis */
    TArrow* m_arrowx = nullptr; /**< x-axis direction */
    TArrow* m_arrowy = nullptr; /**< y-axis direction */

    TPaveText* m_legProblem = nullptr; /**< efficiency plot legend, problem */
    TPaveText* m_legWarning = nullptr; /**< efficiency plot legend, warning */
    TPaveText* m_legNormal = nullptr; /**< efficiency plot legend, normal */
    TPaveText* m_legEmpty = nullptr; /**< efficiency plot legend, empty */

    int m_efficiencyMin = 0; /**< Minimum of the efficiency histogram */
    int m_efficiencyMax = -1111; /**< Maximum of the efficiency histogram. -1111 adjust the maximum depennding on the content */
    bool m_setEfficiencyRange = false; /**< set the range of the efficiency histogram */

    /** efficiency status flags */
    enum effStatus {
      good = 0,    /**< green frame */
      warning = 1, /**< orange frame */
      error = 2,   /**< red frame */
      lowStat = 3  /**< gray frame */
    };
    effStatus m_effUstatus; /**< number representing the status of the efficiency U side */
    effStatus m_effVstatus;/**< number representing the status of the efficiency V side */

    //! IDs of all SVD Modules to iterate over
    std::vector<VxdID> m_SVDModules;

    std::string m_pvPrefix; /**< string prefix for EPICS PVs */


  };
} // end namespace Belle2

