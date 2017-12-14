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

    /** Return singleton instance of the statistics */
    static ProcessStatisticsPython& getInstance();

    /** Get wrapped ProcessStatistics object. */
    ProcessStatistics* getWrapped();

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
                                    const std::vector<ModuleStatistics>* modules = nullptr);

    /**
     * Return string with statistics for selected modules
     * @param modulesPyList python list of modules to show
     * @param type   counter type to use for statistics
     */
    std::string getModuleStatistics(const boost::python::list& modulesPyList,
                                    ModuleStatistics::EStatisticCounters type = ModuleStatistics::c_Event);

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
    const ModuleStatistics* get(std::shared_ptr<Module> module);

    /** Get statistics for the framework itself. */
    const ModuleStatistics* getGlobal();

    /** Get statistics for all modules as python list */
    boost::python::list getAll();

    /** Clear collected statistics but keep names of modules */
    void clear();

    /** Define python wrappers to make functionality avaiable in python */
    static void exposePythonAPI();
  private:
    /** Private constructor due to singleton pattern */
    ProcessStatisticsPython() = default;
    /** Prohibit copy constructor */
    ProcessStatisticsPython(const ProcessStatisticsPython&) = delete;
    /** Prohibit assignment operator */
    ProcessStatisticsPython& operator=(ProcessStatisticsPython&) = delete;

  };
} //Belle2 namespace
