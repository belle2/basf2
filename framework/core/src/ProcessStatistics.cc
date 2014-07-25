/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/Module.h>
#include <framework/core/ProcessStatistics.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <sstream>

using namespace std;
using namespace Belle2;

ClassImp(ProcessStatistics);

void ProcessStatistics::initModule(Module* module)
{
  ModuleStatistics& stats = m_stats[getIndex(module)];
  if (stats.getName().empty()) {
    stats.setName(module->getName());
  }
  stats.setIndex(m_stats.size() - 1);
}

string ProcessStatistics::getStatisticsString(ModuleStatistics::EStatisticCounters mode, const ProcessStatistics::StatisticsMap* modules)
{
  if (!modules) modules = &(getAll());
  stringstream out;
  int moduleNameLength = 21; //minimum: 80 characters
  const int lengthOfRest = 80 - moduleNameLength;
  for (const ModuleStatistics & stats : *modules) {
    int len = stats.getName().length();
    if (len > moduleNameLength)
      moduleNameLength = len;
  }
  std::string numTabsModule = (boost::format("%d") % (moduleNameLength + 1)).str();
  std::string numWidth = (boost::format("%d") % (moduleNameLength + 1 + lengthOfRest)).str();
  out << boost::format("%|" + numWidth + "T=|\n");
  boost::format outputheader("%s %|" + numTabsModule + "t|| %10s | %10s | %10s | %17s\n");
  boost::format output("%s %|" + numTabsModule + "t|| %10.0f | %10.0f | %10.2f | %7.2f +-%7.2f\n");
  out << outputheader % "Name" % "Calls" % "Memory(MB)" % "Time(s)" % "Time(ms)/Call";
  out << boost::format("%|" + numWidth + "T=|\n");

  const int numModules = modules->size();
  for (int iModule = 0; iModule < numModules; iModule++) {
    //find Module with ID = iModule, to list them by initialisation
    for (const ModuleStatistics & stats : *modules) {
      if (stats.getIndex() != iModule)
        continue;

      out << output
          % stats.getName()
          % stats.getCalls(mode)
          % (stats.getMemorySum(mode) / 1024)
          % (stats.getTimeSum(mode) / Unit::s)
          % (stats.getTimeMean(mode) / Unit::ms)
          % (stats.getTimeStddev(mode) / Unit::ms);
    }
  }

  out << boost::format("%|" + numWidth + "T=|\n");
  ModuleStatistics& global = getGlobal();
  out << output
      % "Total"
      % global.getCalls(mode)
      % (global.getMemorySum(mode) / 1024)
      % (global.getTimeSum(mode) / Unit::s)
      % (global.getTimeMean(mode) / Unit::ms)
      % (global.getTimeStddev(mode) / Unit::ms);
  out << boost::format("%|" + numWidth + "T=|\n");
  return out.str();
}

void ProcessStatistics::merge(const Mergeable* other)
{
  const ProcessStatistics* otherObject = static_cast<const ProcessStatistics*>(other);

  B2WARNING("this:");
  B2WARNING(this->getStatisticsString());
  /*
  for (const auto mapEntry : this->m_modules) {
    B2INFO("ptr " << mapEntry.first << " \t " << mapEntry.second.getIndex() << " \t" << mapEntry.second.getName());
  }
  */

  B2WARNING("other:");
  B2WARNING(const_cast<ProcessStatistics*>(otherObject)->getStatisticsString());
  /*
  for (const auto mapEntry : otherObject->m_modules) {
    B2INFO("ptr " << mapEntry.first << " \t " << mapEntry.second.getIndex() << " \t" << mapEntry.second.getName());
  }
  */

  m_global.update(otherObject->m_global);

  for (const auto otherStats : otherObject->m_stats) {
    for (auto & myStats : this->m_stats) {
      if (otherStats.getIndex() == myStats.getIndex() and otherStats.getName() == myStats.getName()) {
        myStats.update(otherStats);
      }
    }
  }

  //TODO index fudging ?

  B2WARNING("merged stuff:");
  B2WARNING(this->getStatisticsString());

}

void ProcessStatistics::clear()
{
  m_global.clear();
  for (auto & stats : m_stats) { stats.clear(); }
}
