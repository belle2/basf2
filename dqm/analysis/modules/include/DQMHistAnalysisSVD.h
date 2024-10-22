/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisSVDClustersOnTrack.h
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

  protected:
    TPaveText* m_legProblem = nullptr; /**< cluster time on tracks plot legend, problem */
    TPaveText* m_legNormal = nullptr; /**< cluster time on tracks plot legend, normal */
    TPaveText* m_legEmpty = nullptr; /**< cluster time on tracks plot legend, empty */
    TPaveText* m_legWarning = nullptr; /**< cluster time on tracks plot legend, warning */

    TPaveText* m_legOnProblem = nullptr; /**< onlineOccupancy plot legend, problem */
    TPaveText* m_legOnWarning = nullptr; /**< onlineOccupancy plot legend, warning */
    TPaveText* m_legOnNormal = nullptr; /**< onlineOccupancy plot legend, normal */

    /**  status flags */
    enum svdStatus {
      good = 0,    /**< green frame */
      warning = 1, /**< orange frame */
      error = 2,   /**< red frame */
      lowStat = 3,  /**< gray frame */
      noStat = 4 /**< purple frame */
    };

  };
} // end namespace Belle2

