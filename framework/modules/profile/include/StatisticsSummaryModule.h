/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    virtual void initialize() override;

    /** Record begin run statistics sum */
    virtual void beginRun() override;

    /** Record event statistics sum */
    virtual void event() override;

    /** Record end run statistics sum */
    virtual void endRun() override;

  private:
    /** Record the statistics of given type */
    void record(ModuleStatistics::EStatisticCounters type);

    /** process statistics pointer */
    StoreObjPtr<ProcessStatistics> m_processStatistics;
  };
}
