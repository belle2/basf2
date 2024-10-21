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
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisSVDClustersOnTrackModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisSVDClustersOnTrackModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisSVDClustersOnTrackModule();

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
    //! Parameters accesible from basf2 scripts
    bool m_printCanvas; /**< if true print the pdf of the canvases */
    bool m_3Samples; /**< if true enable 3 samples histograms analysis */

    double m_statThreshold; /**< minimal number of events to compare histograms */
    double m_timeThreshold; /**< difference between mean of cluster time for present and reference run */
    float m_refMeanP; /**< mean of the signal time peak from Physics reference run */
    float m_refMeanC; /**< mean of the signal time peak from Cosmic reference run */

    TH1F m_hClusterOnTrackTime_L456V; /**< time for clusters on Track for L456V histo*/
    TCanvas* m_cClusterOnTrackTime_L456V = nullptr; /**< time for clusters on Track for L456V canvas*/

    TH1F m_hClusterOnTrackTimeL456V3Samples; /**< time for clusters on Track for L456V histo for 3 samples*/
    TCanvas* m_cClusterOnTrackTimeL456V3Samples = nullptr; /**< time for clusters on Track for L456V canvas for 3 sampples */


    TPaveText* m_legTiProblem = nullptr; /**< cluster time on tracks plot legend, problem */
    TPaveText* m_legTiNormal = nullptr; /**< cluster time on tracks plot legend, normal */
    TPaveText* m_legTiEmpty = nullptr; /**< cluster time on tracks plot legend, empty */

    TPaveText* m_legTi3Problem = nullptr; /**< cluster time on tracks  for 3 samples plot legend, problem */
    TPaveText* m_legTi3Normal = nullptr; /**< cluster time on tracks for 3 samples  plot legend, normal */
    TPaveText* m_legTi3Empty = nullptr; /**< cluster time on tracks  for 3 samples plot legend, empty */

    TString m_runtype = nullptr; /**< string with runtype: physics or cosmic */

    std::string m_pvPrefix; /**< string prefix for EPICS PVs */

  };
} // end namespace Belle2

