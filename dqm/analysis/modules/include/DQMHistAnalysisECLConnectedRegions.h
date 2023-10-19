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

namespace Belle2 {

  /**
   * This module is for analysis of ECL DQM histograms.
   */

  class DQMHistAnalysisECLConnectedRegionsModule final : public
    DQMHistAnalysisModule { /**< derived from DQMHistAnalysisModule class. */

  public:

    /** Constructor. */
    DQMHistAnalysisECLConnectedRegionsModule();

    /** Destructor. */
    ~DQMHistAnalysisECLConnectedRegionsModule() {};

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

    /** Prefix to use for PVs registered by this module */
    std::string m_pvPrefix;

    /** Average number of crystals in largest ECL CR */
    double m_largestCRAvgCrystalsNum = 0;
    /** Average number local maximum in largest ECL CR */
    double m_largestCRLocalMaxNum = 0;
    /** monitoring object */
    MonitoringObject* m_monObj = nullptr;
  };
} // end namespace Belle2

