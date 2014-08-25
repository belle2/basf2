/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef FRAMEWORK_CORE_PROCESSSTATISTICS_H
#define FRAMEWORK_CORE_PROCESSSTATISTICS_H

#include <framework/core/ModuleStatistics.h>
#include <framework/utilities/Utils.h>

#include <framework/pcore/Mergeable.h>

#include <map>


namespace Belle2 {
  class Module;

  /**
   * Class to collect call statistics for all modules
   *
   * This class is used to collect call and time statistics for all modules.
   * It is implemented as a singleton and will keep track of the number of
   * times a module will be called and the time the module spends in these
   * calls.
   *
   * Altough this class can be used in C++, its main purpose is to be used
   * in python. In the python environment it is reachable through the
   * "statistics" object in the pybasf2 module. Most simple use is to just
   * print the event statistics after the process loop:
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
   * It is also possible to restrict the event statistics to a list of
   * modules one is interested in
   *
   * >>> foo = register_module("Foo")
   * >>> bar = register_module("Bar")
   * ...
   * >>> process(...)
   * >>> print statistics([foo,bar])
   * >>> print statistics([foo,bar],statistics.BEGIN_RUN)
   *
   * More detailed statistics can be reached by accessing the statistics
   * for all modules directly:
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
  class ProcessStatistics : public Mergeable {
  public:
    /** Constructor. */
    ProcessStatistics(): m_global("global"), m_globalTime(0), m_globalMemory(0), m_moduleTime(0), m_moduleMemory(0), m_suspendedTime(0), m_suspendedMemory(0) { }

    /**
     * Return string with statistics for all modules.
     *
     * Can be used in steering file with 'print statistics'.
     *
     * @param type    counter type to use for statistics
     * @param modules map of modules to use. If NULL, default map will be
     *                used
     */
    std::string getStatisticsString(ModuleStatistics::EStatisticCounters type = ModuleStatistics::c_Event,
                                    const std::vector<Belle2::ModuleStatistics>* modules = 0) const;

    /** Get global statistics. */
    ModuleStatistics& getGlobal() { return m_global; }

    /** Get entire statistics map. */
    std::vector<Belle2::ModuleStatistics>& getAll() { return m_stats; }

//We don't want CINT to see anything about Module
#if defined(__CINT__) || defined(__ROOTCLING__) || defined(R__DICTIONARY_FILENAME)
#else

    /** Start timer for global measurement */
    void startGlobal() { setCounters(m_globalTime, m_globalMemory); }

    /** Suspend timer for global measurement, needed for newRun.
     * resumeGlobal should be called once endRun/newRun handling is
     * finished */
    void suspendGlobal() {
      setCounters(m_suspendedTime, m_suspendedMemory,
                  m_globalTime, m_globalMemory);
    }

    /** Resume timer after call to suspendGlobal() */
    void resumeGlobal() {
      setCounters(m_globalTime, m_globalMemory,
                  m_suspendedTime, m_suspendedMemory);
    }

    /** Stop global timer and add values to the statistic counter */
    void stopGlobal(ModuleStatistics::EStatisticCounters type) {
      setCounters(m_globalTime, m_globalMemory,
                  m_globalTime, m_globalMemory);
      m_global.add(type, m_globalTime, m_globalMemory);
    }

    /** Start module timer */
    void startModule() {
      setCounters(m_moduleTime, m_moduleMemory);
    }

    /** Stop module counter and attribute values to appropriate module */
    void stopModule(Module* module, ModuleStatistics::EStatisticCounters type) {
      setCounters(m_moduleTime, m_moduleMemory,
                  m_moduleTime, m_moduleMemory);
      m_stats[getIndex(module)].add(type, m_moduleTime, m_moduleMemory);
    }

    /** Init module statistics: Set name from module if still empty and
     * remember initialization index for display
     */
    void initModule(Module* module);


    /**
     * Get statistics for single module
     * @param module Shared pointer to the Module for which the
     *               statistics should be obtained
     */
    ModuleStatistics& getStatistics(Module* module) {
      return m_stats[getIndex(module)];
    }

    /** get m_stats index for given module, inserting it if not found. */
    int getIndex(Module* module) {
      auto indexIt = m_modulesToStatsIndex.find(module);
      if (indexIt == m_modulesToStatsIndex.end()) {
        int index = m_stats.size();
        m_modulesToStatsIndex[module] = index;
        m_stats.emplace_back();
        return index;
      } else {
        return indexIt->second;
      }
    }
#endif

    /** Merge other ProcessStatistics object into this one. */
    virtual void merge(const Mergeable* other);

    /** Clear collected statistics but keep names of modules */
    virtual void clear();


  private:
    /** Prohibit copy constructor */
    ProcessStatistics(const ProcessStatistics&);
    /** Prohibit assignment operator */
    ProcessStatistics& operator=(ProcessStatistics&);

    /** Set counters time and memory to contain the current clock value
     * and memory consumption respectively.
     * @param time variable to store clock counter
     * @param memory variable to store heap size
     * @param startTime value to subtract from clock counter
     * @param startMemory value to subtract from heap size
     */
    void setCounters(double& time, double& memory,
                     double startTime = 0, double startMemory = 0) {
      time = Utils::getClock() - startTime;
      memory = Utils::getMemoryKB() - startMemory;
    }

    /** Statistics object for global time and memory consumption */
    ModuleStatistics m_global;
    /** module statistics */
    std::vector<Belle2::ModuleStatistics> m_stats;

    std::map<Module*, int> m_modulesToStatsIndex; //!< transient, maps Module* to m_stats index

    //the following are used for the (process-local) time-keeping

    /** store clock counter for global time consumption */
    double m_globalTime; //! (transient)
    /** store heap size for global memory consumption in KB */
    double m_globalMemory; //! (transient)
    /** store clock counter for time consumption by modules */
    double m_moduleTime; //! (transient)
    /** store heap size for memory consumption by modules */
    double m_moduleMemory; //! (transient)
    /** store clock counter for suspended measurement. Generally this
     * would be a stack of values but we know that we need at most one
     * element so we keep it a plain double. */
    double m_suspendedTime; //! (transient)
    /** store heap size for suspended measurement. Generally this
     * would be a stack of values but we know that we need at most one
     * element so we keep it a plain double. */
    double m_suspendedMemory; //! (transient)

    ClassDef(ProcessStatistics, 1); /**< Class to collect call statistics for all modules. */
  };

} //Belle2 namespace
#endif // FRAMEWORK_CORE_PROCESSSTATISTICS_H
