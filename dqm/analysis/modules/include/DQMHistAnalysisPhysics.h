/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPhysics.h
// Description : DQM module, for the physics histograms at hlt level
//-

#pragma once
//DQM
#include <dqm/core/DQMHistAnalysis.h>
#include <TPaveText.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for Physics */

  class DQMHistAnalysisPhysicsModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisPhysicsModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisPhysicsModule();

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
     * End-of-run action.
     */
    void endRun(void) override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate(void) override final;

  private:


    //! TPaveText, Ups Invariant Mass (mumu)
    TPaveText* m_cmUPS_text = nullptr;

    //! TPaveText, Ups ee Invariant Mass
    TPaveText* m_cmUPSe_text = nullptr;

    //! TPaveText, ratios physics results
    TPaveText* m_ratio_text = nullptr;

    //! prefix for EPICS PVs
    std::string m_pvPrefix;

    /** Monitoring Object */
    MonitoringObject* m_monObj {};

  };
} // end namespace Belle2

