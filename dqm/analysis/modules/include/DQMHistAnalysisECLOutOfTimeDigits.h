/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// EPICS
#include "cadef.h"

//DQM
#include <dqm/core/DQMHistAnalysis.h>

namespace Belle2 {

  /**
   * This module is for analysis of ECL DQM histograms.
   */

  class DQMHistAnalysisECLOutOfTimeDigitsModule final : public
    DQMHistAnalysisModule { /**< derived from DQMHistAnalysisModule class. */

  public:

    /** Constructor. */
    DQMHistAnalysisECLOutOfTimeDigitsModule();

    /** Destructor. */
    ~DQMHistAnalysisECLOutOfTimeDigitsModule() {}

    /** Initialize the module. */
    void initialize() override final;
    /** Event processor. */
    void event() override final;
    /** Call when a run ends. */
    void endRun() override final;
    /** Terminate. */
    void terminate() override final {}

  private:
    /**  Out-of-time ECLCalDigits for several cases */
    std::map<std::string, double> m_out_of_time_digits = {};

    /** Prefix to use for PVs registered by this module */
    std::string m_pvPrefix;

    MonitoringObject* m_monObj = nullptr; /**< monitoring object */
  };
} // end namespace Belle2

