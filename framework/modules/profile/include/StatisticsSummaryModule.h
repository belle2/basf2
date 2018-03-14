/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/ProcessStatistics.h>


namespace Belle2 {
  /**
   * A module that displays the sum of multiple modules in the module statistics.
   *
   * This module sums up the the statistics of all previous modules up to the
   * first or another TimingSummary module in the module statistis list and
   * then sets its own statistics to the sum.
   *
   */
  class StatisticsSummaryModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description of the module.
     */
    StatisticsSummaryModule();

    /** Record initialize statistics sum */
    virtual void initialize();

    /** Record begin run statistics sum */
    virtual void beginRun();

    /** Record event statistics sum */
    virtual void event();

    /** Record end run statistics sum */
    virtual void endRun();

  private:
    /** Record the statistics of given type */
    void record(ModuleStatistics::EStatisticCounters type);

    /** process statistics pointer */
    StoreObjPtr<ProcessStatistics> m_processStatistics;
  };
}
