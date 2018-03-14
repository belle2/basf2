/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/profile/StatisticsSummaryModule.h>

#include <framework/core/ModuleStatistics.h>
#include <framework/core/ModuleManager.h>


using namespace Belle2;

// Register the Module
REG_MODULE(StatisticsSummary)


StatisticsSummaryModule::StatisticsSummaryModule() : Module(), m_processStatistics("", DataStore::c_Persistent)
{
  // Set module description
  setDescription("Sums up the statistics of preceeding modules. All modules until the first module or another StatisticsSummary module in the module statistics are included.");
  setPropertyFlags(c_ParallelProcessingCertified | c_DontCollectStatistics);
}

void StatisticsSummaryModule::initialize()
{
  record(ModuleStatistics::c_Init);
}

void StatisticsSummaryModule::beginRun()
{
  record(ModuleStatistics::c_BeginRun);
}

void StatisticsSummaryModule::event()
{
  record(ModuleStatistics::c_Event);
}

void StatisticsSummaryModule::endRun()
{
  record(ModuleStatistics::c_EndRun);
}

void StatisticsSummaryModule::record(ModuleStatistics::EStatisticCounters type)
{
  // get module statistics and list of modules
  auto allStatistics = m_processStatistics->getAll();
  auto modules = ModuleManager::Instance().getCreatedModules();

  // initialize sums
  ModuleStatistics::value_type time = 0;
  ModuleStatistics::value_type memory = 0;

  // loop over module statistics
  for (int index = m_processStatistics->getIndex(this) - 1; index >= 0; index--) {
    const ModuleStatistics& statistics = allStatistics[index];

    // check if we have another StatisticsSummary module
    std::string name = statistics.getName();
    const Module* module = 0;
    for (auto aModule : modules) {
      if (aModule->getName() == name) {
        module = aModule.get();
        break;
      }
    }
    if (module && (module->getType() == "StatisticsSummary")) {
      break;
    }

    // sum up
    time += statistics.getTimeSum(type);
    memory += statistics.getMemorySum(type);
  }

  // update statistics of this module
  ModuleStatistics& thisStatistics = m_processStatistics->getStatistics(this);
  time -= thisStatistics.getTimeSum(type);
  memory -= thisStatistics.getMemorySum(type);
  thisStatistics.add(type, time, memory);
}

