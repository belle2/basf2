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

// Monitoring
#include <dqm/dataobjects/DQMFileMetaData.h>

namespace Belle2 {

  /**
   * Make summary of data quality from reconstruction
   */
  class DQMHistAnalysisMiraBelleModule final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisMiraBelleModule();

    /**
     * Destructor
     */
    ~DQMHistAnalysisMiraBelleModule();

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
     * Event processor.
     */
    void event() override final;

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

  protected:
    double m_scale_dst; /**< Scale factor "signal region" / "sideband", assuming uniform events */
    double reference_hadb; /**< Reference value for hadronb2_tight bhabha ratio */
    double reference_mumu; /**< Reference value for mumu inv mass */
    Belle2::MonitoringObject* mon_mumu = nullptr; /**< MonitoringObject for mumu events */
    Belle2::MonitoringObject* mon_dst = nullptr; /**< MonitoringObject for D* events */
    Belle2::MonitoringObject* mon_bhabha = nullptr; /**< MonitoringObject for bhabha events */
    Belle2::MonitoringObject* mon_hadron = nullptr; /**< MonitoringObject for hadronb2 tight events */

    // TCanvases for mumu
    TCanvas* mumu_main = nullptr; /**< TCanvas object for main display */
    TCanvas* mumu_resolution = nullptr; /**< TCanvas object for resolution display */
    TCanvas* mumu_muon_val = nullptr; /**< TCanvas object for mumu values display */
    // TCanvases for D*
    TCanvas* dst_mass = nullptr; /**< TCanvas object for D0 mass, delta mass, and pi0 mass */
    TCanvas* dst_pi_val = nullptr; /**< TCanvas object for PID distributions of pi */
    TCanvas* dst_k_val = nullptr; /**< TCanvas object for PID distributions of K */
    // TCanvases for bhabha
    TCanvas* bhabha_main = nullptr; /**< TCanvas object for main display */
    TCanvas* bhabha_resolution = nullptr; /**< TCanvas object for resolution display */
    // TCanvases for hadronb2 tight
    TCanvas* hadron_main = nullptr; /**< TCanvas object for main display */

  };

} // Belle2 namespace
