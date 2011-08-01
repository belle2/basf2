/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MODULESTATISTICS_H
#define MODULESTATISTICS_H

#include <map>
#include <cstring>
#include <string>
#include <numeric>
#include <framework/gearbox/Unit.h>
#include <framework/core/Module.h>
#include <framework/core/utilities.h>
#include <boost/python.hpp>
#include <boost/array.hpp>

namespace Belle2 {

  /**
   * Class to collect call statistics for all modules
   *
   * This class is used to collect call and time statistics for all modules.
   * It is implemented as a singleton and will keep track of the number of
   * times a module will be called and the time the module spends in these
   * calls.
   *
   * Altough this class can be used in C++, its main purpose is to be used in
   * python. In the python environment it is reachable through the "statistics"
   * object in the pybasf2 module. Most simple use is to just print the event
   * statistics after the process loop:
   *
   * >>> process(...)
   * >>> print statistics
   *
   * Different types of statistics can be printed using
   *
   * >>> print statistics(type)
   *
   * where type can be one of
   *  - statistics.INIT        -> time/calls spent in initialize()
   *  - statistics.BEGIN_RUN   -> time/calls spent in beginRun()
   *  - statistics.EVENT       -> time/calls spent in event()
   *  - statistics.END_RUN     -> time/calls spent in endRun()
   *  - statistics.TERM        -> time/calls spent in terminate()
   *  - statistics.TOTAL       -> sum of all the above
   *
   * It is also possible to restrict the event statistics to a list of modules
   * one is interested in
   *
   * >>> foo = register_module("Foo")
   * >>> bar = register_module("Bar")
   * ...
   * >>> process(...)
   * >>> print statistics([foo,bar])
   * >>> print statistics([foo,bar],statistics.BEGIN_RUN)
   *
   * More detailed statistics can be reached by accessing the statistics for
   * all modules directly:
   *
   * >>> process(...)
   * >>> for stats in statistics.modules:
   * >>>     print stats.name, stats.time(statistics.EVENT), stats.calls(statistics.BEGIN_RUN)
   *
   * Available attributes/methods for the statistics objects are
   *  - name:                         name of the module
   *  - time(type=statistics.EVENT):  time in seconds spent in function
   *  - calls(type=statistics.EVENT): number of calls to function
   *
   * The global statistics for the framework can be accessed via
   * statistics.framework
   *
   * The name shown in the statistics can be modified. This is particular
   * useful if there is more than one instance of a given module in the path
   *
   * >>> foo = register_module("Foo")
   * >>> statistics.set_name(foo,"Footastic")
   */
  class ModuleStatistics {
  public:
    /** Enum to define all counter types */
    enum ECounters {
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

    /** Class to hold statistic information per module */
    class Statistics {
    public:
      /** Default constructor to reset values */
      Statistics(const std::string& name = ""): m_name(name) { clear(); }
      /** Clear all counters */
      void clear() {
        memset(m_times.c_array(), 0, m_times.size()*sizeof(double));
        memset(m_calls.c_array(), 0, m_calls.size()*sizeof(unsigned int));
      }
      /** Get name of statistics object */
      std::string getName() const { return m_name; }
      /**
       * Get accumulated time for a given counter type
       * @param type which counter to return. c_Total returns the sums of all other counters
       */
      double getTime(ECounters type = c_Event) const {
        if (type >= c_Total) return std::accumulate(m_times.begin(), m_times.end(), 0.0);
        return m_times[type];
      }
      /**
       * Get accumulated calls for a given counter type
       * @param type which counter to return. c_Total returns the sums of all other counters
       */
      double getCalls(ECounters type = c_Event) const {
        if (type >= c_Total) return std::accumulate(m_calls.begin(), m_calls.end(), 0.0);
        return m_calls[type];
      }
    protected:
      /** Name of the module */
      std::string m_name;
      /** Time spent in functions */
      boost::array<double, c_Total> m_times;
      /** Number of calls to functions */
      boost::array<unsigned int, c_Total> m_calls;

      friend class ModuleStatistics;
    };

    /** Container type to hold information for all modules */
    typedef std::map<const Module*, Statistics> StatisticsMap;

    /** Return instance reference */
    static ModuleStatistics& getInstance();

    /**
     * Start to measure global time
     * @param mode    Which counter to increase
     */
    void startGlobal(ECounters mode = c_Event) {
      m_globalStart = Utils::getClock();
    }

    /**
     * Stop to measure global time
     * @param mode    Which counter to increase
     * @param suspend If true, time will be increased but call count will remain unchanged
     */
    void stopGlobal(ECounters mode = c_Event, bool suspend = false);

    /**
     * Start to measure time
     * @param module Module the time will be attributed to
     * @param mode   Which counter to increase
     */
    void startModule(const Module &module, ECounters mode = c_Event) {
      m_moduleStart = Utils::getClock();
    }

    /**
     * Stop to measure time
     * @param module Module the time will be attributed to
     * @param mode   Which counter to increase
     * @param suspend If true, time will be increased but call count will remain unchanged
     */
    void stopModule(const Module &module, ECounters mode = c_Event, bool suspend = false);

    /**
     * Return string with statistics for all modules
     * @param type    counter type to use for statistics
     * @param modules map of modules to use. If NULL, default map will be used
     */
    std::string getStatistics(ECounters type = c_Event, StatisticsMap *modules = 0);

    /**
     * Return string with statistics for selected modules
     * @param pyList python list of modules to show
     * @param type    counter type to use for statistics
     */
    std::string getModuleStatistics(const boost::python::list& pyList, ECounters type = c_Event);

    /**
     * Set name for module in statistics.
     *
     * Normally, all modules get assigned their default name which is
     * used to register them. If multiple instances of the same module
     * are present at the same time, this can be used to distuingish
     * between them
     *
     * @param module Shared pointer to the Module for which a name is
     *               to be defined
     * @param name   Name to show in statistics
     */
    void setModuleName(const ModulePtr &module, const std::string &name) {
      m_modules[module.get()].m_name = name;
    }

    /**
     * Get statistics for single module
     * @param module Shared pointer to the Module for which the
     *               statistics should be obtained
     */
    const Statistics &get(const ModulePtr &module) {
      return m_modules[module.get()];
    }

    /** Get statistics for all modules as python list */
    boost::python::list getAll();

    /** Clear collected statistics, but keep names of modules */
    void clear();

    /** Define python wrappers to make functionality avaiable in python */
    static void exposePythonAPI();

  protected:
    /** Default constructor */
    ModuleStatistics(): m_moduleStart(0), m_globalStart(0), m_global("_global_") {}
    /** Singleton, hide copy constructor*/
    ModuleStatistics(const ModuleStatistics &b) {}
    /** Singleton, hide assignment operator*/
    ModuleStatistics& operator=(ModuleStatistics &b) { return *this; }

    double m_moduleStart;
    double m_globalStart;
    /** Statistics for whole framework */
    Statistics m_global;
    /** Statistical information for all Modules */
    StatisticsMap m_modules;
  };

  /** Increase counters. Defined as inline to save some context switches */
  inline void ModuleStatistics::stopModule(const Module &module, ECounters mode, bool suspend)
  {
    Statistics &stats = m_modules[&module];
    double elapsed = (Utils::getClock() - m_moduleStart) / Unit::s;
    if (mode == c_Init && stats.m_name.empty()) stats.m_name = module.getName();
    stats.m_times[mode] += elapsed;
    if (!suspend) ++stats.m_calls[mode];
  }

  /** Increase counters. Defined as inline to save some context switches */
  inline void ModuleStatistics::stopGlobal(ECounters mode, bool suspend)
  {
    double elapsed = (Utils::getClock() - m_globalStart) / Unit::s;
    m_global.m_times[mode] += elapsed;
    if (!suspend) ++m_global.m_calls[mode];
  }

} //end namespace Belle2

#endif //MODULESTATISTICS_H
