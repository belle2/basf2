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

  class DQMHistAnalysisECLClustersModule final : public DQMHistAnalysisModule { /**< derived from DQMHistAnalysisModule class. */

  public:

    /** Constructor. */
    DQMHistAnalysisECLClustersModule();

    /** Destructor. */
    ~DQMHistAnalysisECLClustersModule() {};

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

    /** Number of bad ECL Clusters */
    double m_badClustersNumber = 0;
    /** Average number of crestals in ECL Clusters */
    double m_avgClusterCrystalsNum = 0;
    /**< monitoring object */
    MonitoringObject* m_monObj = nullptr;
    /**< main panel for monitoring object */
    TCanvas* m_c_main = nullptr;
  };
} // end namespace Belle2

