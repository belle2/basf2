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

  class DQMHistAnalysisECLShapersModule final : public DQMHistAnalysisModule { /**< derived from DQMHistAnalysisModule class. */

  public:

    /** Constructor. */
    DQMHistAnalysisECLShapersModule();

    /** Destructor. */
    ~DQMHistAnalysisECLShapersModule();

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

    /** Number of ECLCollector modules (normally 52) */
    const static int c_collector_count = ECL::ECL_CRATES;

    /** Remove upper 10% of the values, return the maximum in the remaining 90% */
    double robust_max(std::multiset<double> values);

#ifdef _BELLE2_EPICS
    /** EPICS channels for logic_summary histogram */
    chid chid_logic[c_collector_count];
    /** EPICS channels for pedestal width */
    chid chid_pedwidth[4];

    /** Max pedestal width array
     * [0] -> Max pedestal width in FWD endcap
     * [1] -> Max pedestal width in barrel
     * [2] -> Max pedestal width in BWD endcap
     * [3] -> Max pedestal width in all calorimeter
     * The algorithm takes not the actual maximum,
     * it first filters out highest 10% of noisy channels
     * in an attempt to provide more robust estimate.
     */
    double m_pedwidth_max[4] = {};

    MonitoringObject* m_monObj = nullptr; /**< monitoring object */
    TCanvas* m_c_main = nullptr; /**< main panel for monitoring object */
#endif
  };
} // end namespace Belle2

