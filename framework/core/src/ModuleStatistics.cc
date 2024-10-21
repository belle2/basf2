/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/core/ModuleStatistics.h>

using namespace Belle2;

void ModuleStatistics::csv_header(std::ostream& output) const
{
  output << "name";
  for (auto resource : {"time", "memory"}) {
    for (auto type : {"init", "begin_run", "event", "end_run", "term", "total"}) {
      output << "," << resource << " " << type;
    }
    output << "," << resource << " mean" << "," << resource << " stddev";
  }
  output << std::endl;
}

void ModuleStatistics::csv(std::ostream& output) const
{
  output << m_name;

  for (EStatisticCounters type = c_Init; type <= c_Total; type = EStatisticCounters(type + 1)) {
    output << "," << m_stats[type].getSum<0>();
  }
  output << "," << m_stats[c_Event].getMean<0>() << ","  << m_stats[c_Event].getStddev<0>();

  for (EStatisticCounters type = c_Init; type <= c_Total; type = EStatisticCounters(type + 1)) {
    output << "," << m_stats[type].getSum<1>();
  }
  output << "," << m_stats[c_Event].getMean<1>() << ","  << m_stats[c_Event].getStddev<1>();

  output << std::endl;
}