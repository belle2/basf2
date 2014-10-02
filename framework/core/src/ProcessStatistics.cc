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

#include <algorithm>
#include <sstream>

using namespace std;
using namespace Belle2;

ClassImp(ProcessStatistics);

int ProcessStatistics::getIndex(const Module* module)
{
  auto indexIt = m_modulesToStatsIndex.find(module);
  if (indexIt == m_modulesToStatsIndex.end()) {
    int index = m_stats.size();
    m_modulesToStatsIndex[module] = index;
    m_stats.emplace_back();
    initModule(module);
    return index;
  } else {
    return indexIt->second;
  }
}
void ProcessStatistics::initModule(const Module* module)
{
  int index = getIndex(module);
  ModuleStatistics& stats = m_stats.at(index);
  if (module and stats.getName().empty()) {
    stats.setName(module->getName());
  }
  stats.setIndex(index);
}

string ProcessStatistics::getStatisticsString(ModuleStatistics::EStatisticCounters mode, const std::vector<ModuleStatistics>* modules) const
{
  const ModuleStatistics& global = getGlobal();
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

  std::vector<ModuleStatistics> modulesSortedByIndex(*modules);
  sort(modulesSortedByIndex.begin(), modulesSortedByIndex.end(), [](const ModuleStatistics & a, const ModuleStatistics & b) { return a.getIndex() < b.getIndex(); });
  for (const ModuleStatistics & stats : modulesSortedByIndex) {
    out << output
        % stats.getName()
        % stats.getCalls(mode)
        % (stats.getMemorySum(mode) / 1024)
        % (stats.getTimeSum(mode) / Unit::s)
        % (stats.getTimeMean(mode) / Unit::ms)
        % (stats.getTimeStddev(mode) / Unit::ms);
  }

  out << boost::format("%|" + numWidth + "T=|\n");
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

  m_global.update(otherObject->m_global);

  if (m_stats.size() == otherObject->m_stats.size()) {
    //fast version for merging between processes
    for (unsigned int i = 0; i < otherObject->m_stats.size(); i++) {
      ModuleStatistics& myStats = m_stats[i];
      const ModuleStatistics& otherStats = otherObject->m_stats[i];
      if (myStats.getName() == otherStats.getName()) {
        myStats.update(otherStats);
      } else {
        B2ERROR("mismatch in module names in statistics. ProcessStatistics::merge() can only merge statistics that contain exactly the same modules.");
      }
    }
  } else {
    //slow merging if we have different number of modules
    for (const auto & otherStats : otherObject->m_stats) {
      //find name in our statistics
      bool found = false;
      for (ModuleStatistics & myStats : m_stats) {
        if (myStats.getName() == otherStats.getName()) {
          myStats.update(otherStats);
          found = true;
          break;
        }
      }

      if (!found) {
        //add module at end
        m_stats.emplace_back(otherStats);
        m_stats.back().setIndex(m_stats.size() - 1);
      }
    }
  }
}

void ProcessStatistics::clear()
{
  m_global.clear();
  for (auto & stats : m_stats) { stats.clear(); }
}

void ProcessStatistics::setCounters(double& time, double& memory,
                                    double startTime, double startMemory)
{
  time = Utils::getClock() - startTime;
  memory = Utils::getMemoryKB() - startMemory;
}
