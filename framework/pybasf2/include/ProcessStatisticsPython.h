/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Christian Pulvermacher                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/ModuleStatistics.h>
#include <framework/core/ProcessStatistics.h>

namespace boost {
  namespace python {
    class list;
  }
}

namespace Belle2 {
  class Module;

  /** Python interface for ProcessStatistics.
   *
   * Since ProcessStatistics itself resides in the data store, this wraps around it to export some functionality to python.
   */
  class ProcessStatisticsPython {
  public:
    /** Construct a new object to show statistics for a given call type. Possibly also restricting the list of modules to be shown */
    ProcessStatisticsPython(ModuleStatistics::EStatisticCounters type = ModuleStatistics::c_Event,
                            const std::vector<ModuleStatistics>& modules = {}): m_type{type}, m_modules{modules} {}
    /** Default copy constructor. */
    ProcessStatisticsPython(const ProcessStatisticsPython&) = default;
    /** Default assignment operator. */
    ProcessStatisticsPython& operator=(const ProcessStatisticsPython&) = default;
    /** Default destructor */
    ~ProcessStatisticsPython() = default;

    /** Get wrapped ProcessStatistics object. */
    ProcessStatistics* getWrapped();

    /**
     * Return string with statistics for all selected modules.
     * If none are selected show all modules
     *
     * Can be used in steering file with 'print(statistics)'.
     */
    std::string getStatisticsString();

    /**
     * Return string with statistics for all selected modules as html table.
     * If none are selected show all modules
     *
     * Is used in Jupyter notebooks
     */
    std::string getStatisticsStringHTML();

    /** Get a new statistics object for a different counter/different list of modules */
    ProcessStatisticsPython getModuleStatistics(ModuleStatistics::EStatisticCounters type, const boost::python::list& modulesPyList);

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
    void setModuleName(Module* module, const std::string& name)
    {
      getWrapped()->getStatistics(module).setName(name);
    }

    /** Get statistics for given module. */
    const ModuleStatistics* get(const std::shared_ptr<Module>& module);

    /** Get statistics for the framework itself. */
    const ModuleStatistics* getGlobal();

    /** Get statistics for all modules as python list */
    boost::python::list getAll();

    /** Clear collected statistics but keep names of modules */
    void clear();

    /** Define python wrappers to make functionality avaiable in python */
    static void exposePythonAPI();
  private:
    /** Which counter to show when printing the statistics */
    ModuleStatistics::EStatisticCounters m_type{ModuleStatistics::c_Event};
    /** Which modules to show. If empty, show all modules */
    std::vector<ModuleStatistics> m_modules;
  };
} //Belle2 namespace
