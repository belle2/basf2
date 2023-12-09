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
#include <TCanvas.h>

namespace Belle2 {

  /**
   * Example module of how to use MonitoringObject in DQMHistAnalysis module
   */
  class DQMHistAnalysisEpicsOutputModule final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisEpicsOutputModule();

    /**
     * Initialize the Module.
     */
    void initialize(void) override final;

    /**
     * Event action.
     */
    void event(void) override final;

    /**
     * End run action.
     */
    void beginRun(void) override final;

    /**
     * End run action.
     */
    void endRun(void) override final;

  protected:

  };

} // Belle2 namespace
