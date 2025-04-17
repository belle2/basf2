/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisSVDModule.h
// Description : base module for DQM histogram analysis of SVD
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
#include <TText.h>
#include <TLine.h>
#include <TArrow.h>
#include <TString.h>

namespace Belle2 {
  /*! Class definition for common method */

  class  DQMHistAnalysisSVDModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisSVDModule(bool panelTop = false, bool online = false);

    /**
     * Destructor.
     */
    ~ DQMHistAnalysisSVDModule();

    void setStatusOfCanvas(int status, TCanvas* canvas, bool plotLeg = true, bool online = false); /**< set status of Canvas */
    TString getHistoNameFromCanvas(TString cName, TString view = "",
                                   TString cPrefix = "c_", TString hPrefix = ""); /**< get histogram name from Canvas name*/

  protected:
    TPaveText* m_legProblem = nullptr;  /**< plot legend, problem */
    TPaveText* m_legNormal = nullptr;   /**< plot legend, normal */
    TPaveText* m_legLowStat = nullptr;    /**< plot legend, low stats */
    TPaveText* m_legEmpty = nullptr;    /**< plot legend, empty */
    TPaveText* m_legWarning = nullptr;  /**< plot legend, warning */

    TPaveText* m_legOnlineProblem = nullptr; /**< onlineOccupancy plot legend, problem */
    TPaveText* m_legOnlineWarning = nullptr; /**< onlineOccupancy plot legend, warning */
    TPaveText* m_legOnlineNormal = nullptr;  /**< onlineOccupancy plot legend, normal */

    std::pair<std::vector<TText*>, std::vector<TText*>> textModuleNumbers(); /**< create vectors of TText to write on the canvas */
    void drawText(); /**< draw text on the RPhi view */

    std::vector<TText*> m_laddersText; /**< list of ladders to write on the canvas */
    std::vector<TText*> m_sensorsText; /**< list of sensors to write on the cancas */

    TLine* m_lx = nullptr; /**< y-axis */
    TLine* m_ly = nullptr; /**< x-axis */
    TArrow* m_arrowx = nullptr; /**< x-axis direction */
    TArrow* m_arrowy = nullptr; /**< y-axis direction */

    int m_colzMinimum = 0; /**< Minimum of the  histogram */
    int m_colzMaximum = -1111; /**< Maximum of the  histogram. -1111 adjust the maximum depennding on the content */
    bool m_setColzRange = false; /**< set the range of the  histogram in colz*/

    float m_valueMinimum = -1; /**< Minimum value of parameter  */
    float m_valueMaximum = -1; /**< Maxiimum value of parameter  */

    /**  status flags */
    enum svdStatus {
      good = 0,    /**< green frame */
      warning = 1, /**< orange frame */
      error = 2,   /**< red frame */
      noStat = 3, /**< purple frame */
      lowStat = 4  /**< gray frame */
    };

    void updateCanvases(SVDSummaryPlots* histo, TCanvas* canvas, TCanvas* canvasRPhi, svdStatus status, bool isU,
                        bool online = false); /**< update canvases */
    void updateErrCanvases(SVDSummaryPlots* histo, TCanvas* canvas, TCanvas* canvasRPhi, bool isU); /**< update error canvases */

  };
} // end namespace Belle2

