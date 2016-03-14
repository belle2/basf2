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
    const string& type = module->getType();
    if (type == "Tx" or type == "Rx")
      stats.setName(type);
    else
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
  boost::format outputheader("%s %|" + numTabsModule + "t|| %10s | %10s | %10s | %17s\n");
  boost::format output("%s %|" + numTabsModule + "t|| %10.0f | %10.0f | %10.2f | %7.2f +-%7.2f\n");

  out << boost::format("%|" + numWidth + "T=|\n");
  out << outputheader % "Name" % "Calls" % "Memory(MB)" % "Time(s)" % "Time(ms)/Call";
  out << boost::format("%|" + numWidth + "T=|\n");

  std::vector<ModuleStatistics> modulesSortedByIndex(*modules);
  sort(modulesSortedByIndex.begin(), modulesSortedByIndex.end(), [](const ModuleStatistics & a, const ModuleStatistics & b) { return a.getIndex() < b.getIndex(); });

  ModuleStatistics::value_type maxcalls = 0;
  for (const ModuleStatistics & stats : modulesSortedByIndex) {
    const ModuleStatistics::value_type calls = stats.getCalls(mode);
    if (calls > maxcalls)
      maxcalls = calls;
    out << output
        % stats.getName()
        % calls
        % (stats.getMemorySum(mode) / 1024)
        % (stats.getTimeSum(mode) / Unit::s)
        % (stats.getTimeMean(mode) / Unit::ms)
        % (stats.getTimeStddev(mode) / Unit::ms);
  }

  out << boost::format("%|" + numWidth + "T=|\n");
  out << output
      % "Total"
      % maxcalls
      % (global.getMemorySum(mode) / 1024)
      % (global.getTimeSum(mode) / Unit::s)
      % (global.getTimeMean(mode) / Unit::ms)
      % (global.getTimeStddev(mode) / Unit::ms);
  out << boost::format("%|" + numWidth + "T=|\n");
  return out.str();
}

void ProcessStatistics::appendUnmergedModules(const ProcessStatistics* otherObject)
{
  int minIndexUnmerged = 0;
  if (otherObject->m_modulesToStatsIndex.empty()) {
    B2ERROR("ProcessStatistics::appendUnmergedModules(): Module -> index list is empty? This might produce wrong results");
  } else {
    minIndexUnmerged = otherObject->m_modulesToStatsIndex.begin()->second;
    for (auto pair : otherObject->m_modulesToStatsIndex) {
      if (pair.second < minIndexUnmerged)
        minIndexUnmerged = pair.second;
    }
  }
  if (minIndexUnmerged > m_stats.size())
    B2FATAL("(minIndexUnmerged > m_stats.size()) :( ");
  if (minIndexUnmerged > otherObject->m_stats.size())
    B2FATAL("(minIndexUnmerged > otherObject->m_stats.size()) :( ");


  //the first minIndexUnmerged entries in m_stats should just be merged...
  for (int i = 0; i < minIndexUnmerged; i++) {
    ModuleStatistics& myStats = m_stats[i];
    const ModuleStatistics& otherStats = otherObject->m_stats[i];
    if (myStats.getName() == otherStats.getName()) {
      myStats.update(otherStats);
    } else {
      B2ERROR("mismatch in module names in statistics (" << myStats.getName() << " vs. " << otherStats.getName() << "). ProcessStatistics::merge() can only merge statistics that contain exactly the same modules.");
    }
  }

  //append the rest
  for (int i = minIndexUnmerged; i < otherObject->m_stats.size(); i++) {
    const ModuleStatistics& otherStats = otherObject->m_stats[i];
    m_stats.emplace_back(otherStats);
    m_stats.back().setIndex(m_stats.size() - 1);
  }
  //copy m_modulesToStatsIndex
  //shift indices by #entries missing in otherObject
  const int shift = m_stats.size() - otherObject->m_stats.size();
  if (shift < 0) {
    B2FATAL("shift negative: " << shift);
  }
  for (auto pair : otherObject->m_modulesToStatsIndex) {
    m_modulesToStatsIndex[pair.first] = pair.second + shift;
  }
}


void ProcessStatistics::merge(const Mergeable* other)
{
  const ProcessStatistics* otherObject = static_cast<const ProcessStatistics*>(other);

  m_global.update(otherObject->m_global);

  if (m_stats == otherObject->m_stats) {
    //fast version for merging between processes
    for (unsigned int i = 0; i < otherObject->m_stats.size(); i++)
      m_stats[i].update(otherObject->m_stats[i]);
  } else {
    appendUnmergedModules(otherObject);
  }

  //if the other object has transient data on modules, copy remaining counters
  if (!otherObject->m_modulesToStatsIndex.empty())
    setTransientCounters(otherObject);
}

void ProcessStatistics::setTransientCounters(const ProcessStatistics* otherObject)
{
  m_globalTime = otherObject->m_globalTime;
  m_globalMemory = otherObject->m_globalMemory;
  m_moduleTime = otherObject->m_moduleTime;
  m_moduleMemory = otherObject->m_moduleMemory;
  m_suspendedTime = otherObject->m_suspendedTime;
  m_suspendedMemory = otherObject->m_suspendedMemory;
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

TObject* ProcessStatistics::Clone(const char*) const
{
  ProcessStatistics* p = new ProcessStatistics(*this);
  return p;
}
