/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/utilities/CalcMeanCov.h>
#include <string>

namespace Belle2 {

  /** Keep track of time and memory consumption during processing.
   * This class offers a counter for time and memory consumption for all
   * processing steps (initialize, beginRun, event, endRun, terminate and
   * total). It will automatically calculate a running mean, stddev and
   * correlation factor between time and memory consumption.
   */
  class ModuleStatistics {
  public:
    /** Enum to define all counter types */
    enum EStatisticCounters {
      /** Counting time/calls in initialize() */
      c_Init,
      /** Counting time/calls in beginRun() */
      c_BeginRun,
      /** Counting time/calls in event() */
      c_Event,
      /** Counting time/calls in endRun() */
      c_EndRun,
      /** Counting time/calls in terminate() */
      c_Term,
      /** Sum of the above */
      c_Total
    };

    /** type of float variable to use for calculations and storage */
    typedef double value_type;

    /** Construct with a given name */
    explicit ModuleStatistics(const std::string& name = ""): m_index(0), m_name(name) {}

    /** Add a time and memory measurment to the counter of a given type.
     * @param type Type of counter to add the value to
     * @param time time used during execution
     * @param memory memory size change during execution
     */
    void add(EStatisticCounters type, value_type time, value_type memory)
    {
      m_stats[type].add(time, memory);
      if (type != c_Total)
        m_stats[c_Total].add(time, memory);
    }

    /** Add statistics for each category. */
    void update(const ModuleStatistics& other)
    {
      for (int i = c_Init; i <= c_Total; i++) {
        m_stats[i].add(other.m_stats[i]);
      }
    }

    /** Set the name of the module for display */
    void setName(const std::string& name) { m_name = name; }
    /** Set the index of the module when displaying statistics */
    void setIndex(int index) { m_index = index; }

    /** Return the previously set name */
    const std::string& getName() const { return m_name; }
    /** Return the index */
    int getIndex() const { return m_index; }

    /** return the number of calls for a given counter type */
    value_type getCalls(EStatisticCounters type = c_Total) const
    {
      return m_stats[type].getEntries();
    }

    /** return the sum of all execution times for a given counter */
    value_type getTimeSum(EStatisticCounters type = c_Total) const
    {
      return m_stats[type].getSum<0>();
    }
    /** return the mean execution time for a given counter */
    value_type getTimeMean(EStatisticCounters type = c_Total) const
    {
      return m_stats[type].getMean<0>();
    }
    /** return the stddev of the execution times for a given counter */
    value_type getTimeStddev(EStatisticCounters type = c_Total) const
    {
      return m_stats[type].getStddev<0>();
    }
    /** return the total used memory for a given counter */
    value_type getMemorySum(EStatisticCounters type = c_Total) const
    {
      return m_stats[type].getSum<1>();
    }
    /** return the average memory change per call */
    value_type getMemoryMean(EStatisticCounters type = c_Total) const
    {
      return m_stats[type].getMean<1>();
    }
    /** return the stddev of the memory consumption changes per call */
    value_type getMemoryStddev(EStatisticCounters type = c_Total) const
    {
      return m_stats[type].getStddev<1>();
    }
    /** return the pearson correlation coefficient between execution times
     * and memory consumption changes */
    value_type getTimeMemoryCorrelation(EStatisticCounters type = c_Total) const
    {
      return m_stats[type].getCorrelation<0, 1>();
    }

    /** Check if name is identical. */
    bool operator==(const ModuleStatistics& other) const { return m_name == other.m_name; }
    /** inequality. */
    bool operator!=(const ModuleStatistics& other) const { return !(*this == other); }

    /** Clear all statistics. */
    void clear()
    {
      for (auto& stat : m_stats) stat.clear();
    }
  private:
    /** display index of the module */
    int m_index;
    /** name of module */
    std::string m_name;
    /** array with  mean/covariance for all counters */
    CalcMeanCov<2, value_type> m_stats[c_Total + 1];
  };

} //Belle2 namespace
