/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

//DQM
#include <dqm/core/DQMHistAnalysis.h>

//std
#include <set>

//ECL
#include <ecl/dbobjects/ECLChannelMap.h>

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
    ~DQMHistAnalysisECLOutOfTimeDigitsModule();

    /** Initialize the module. */
    void initialize() override final;
    /** Call when a run begins. */
    void beginRun() override final;
    /** Event processor. */
    void event() override final;
    /** Call when a run ends. */
    void endRun() override final;
    /** Terminate. */
    void terminate() override final;

  private:

    /** flag if to export to EPICS */
    bool m_useEpics;

#ifdef _BELLE2_EPICS
    /** EPICS channels for ECL out of time digits */
    std::map<std::string, chid> chid_out_of_time_digits;
#endif

    /**  Out-of-time ECLCalDigits for several cases */
    std::map<std::string, double> m_out_of_time_digits = {};

    MonitoringObject* m_monObj = nullptr; /**< monitoring object */
    TCanvas* m_c_main = nullptr; /**< main panel for monitoring object */
  };
} // end namespace Belle2

