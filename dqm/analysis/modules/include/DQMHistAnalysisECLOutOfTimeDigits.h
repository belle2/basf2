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

namespace Belle2 {

  /**
   * This module exports the average value of out-of-time ECLCalDigits
   * into EPICS and MiraBelle.
   * See ecl/modules/eclDQMOutOfTimeDigits/include/eclDQMOutOfTimeDigits.h
   * for the motivations that lead to this module development.
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

    /** If true (default), update EPICS PVs only if there were changes in the histograms */
    bool m_onlyIfUpdated;

    MonitoringObject* m_monObj = nullptr; /**< monitoring object */
  };
} // end namespace Belle2

