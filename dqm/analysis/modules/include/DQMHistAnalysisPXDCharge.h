/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPXDCharge.h
// Description : DQM Analysis for PXD Cluster Charges
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#include <vxd/dataobjects/VxdID.h>

#include <TF1.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Cluster Charge */

  class DQMHistAnalysisPXDChargeModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisPXDChargeModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisPXDChargeModule();

    /**
     * Initializer.
     */
    void initialize(void) override final;

    /**
     * Called when entering a new run.
     */
    void beginRun(void) override final;

    /**
     * This method is called for each event.
     */
    void event(void) override final;

    /**
     * This method is called if the current run ends.
     */
    void endRun(void) override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate(void) override final;

  private:
    // Data members
    //! name of histogram directory
    std::string m_histogramDirectoryName;
    //! fit range lo edge for landau
    double m_rangeLow;
    //! fit range hi edge for landau
    double m_rangeHigh;

    //! IDs of all PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    //! only one fit function for all Landaus
    TF1* m_fLandau = nullptr;
    //! Fit the Mean for all modules
    TF1* m_fMean = nullptr;
    //! Histogram covering all modules
    TH1F* m_hCharge = nullptr;
    //! Final Canvas
    TCanvas* m_cCharge = nullptr;

    /** Monitoring Object */
    MonitoringObject* m_monObj {};
  };
} // end namespace Belle2

