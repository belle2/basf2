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
#include <dqm/analysis/modules/DQMHistAnalysis.h>

//std
#include <set>

//ECL
#include <ecl/dbobjects/ECLChannelMap.h>

namespace Belle2 {

  /**
   * This module is for analysis of ECL DQM histograms.
   */

  class DQMHistAnalysisECLShapersModule : public DQMHistAnalysisModule { /**< derived from DQMHistAnalysisModule class. */

  public:

    /** Constructor. */
    DQMHistAnalysisECLShapersModule();

    /** Destructor. */
    virtual ~DQMHistAnalysisECLShapersModule();

    /** Initialize the module. */
    virtual void initialize() override;
    /** Call when a run begins. */
    virtual void beginRun() override;
    /** Event processor. */
    virtual void event() override;
    /** Call when a run ends. */
    virtual void endRun() override;
    /** Terminate. */
    virtual void terminate() override;

  private:

    /** flag if to export to EPICS */
    bool m_useEpics;

    /** Number of ECLCollector modules (normally 52) */
    const static int c_collector_count = ECL::ECL_CRATES;

    /** Remove upper 10% of the values, return the maximum in the remaining 90% */
    double robust_max(std::multiset<double> values);

#ifdef _BELLE2_EPICS
    /** EPICS channels for logic_summary histogram */
    chid chid_logic[c_collector_count];
    /** EPICS channels for pedestal width */
    chid chid_pedwidth[4];
#endif
  };
} // end namespace Belle2

