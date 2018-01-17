/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Christian Pulvermacher                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python.hpp>

#include <framework/pybasf2/ProcessStatisticsPython.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/core/PyObjConvUtils.h>

using namespace Belle2;
using namespace std;
using namespace boost::python;

ProcessStatisticsPython& ProcessStatisticsPython::getInstance()
{
  static ProcessStatisticsPython instance;
  return instance;
}

ProcessStatistics* ProcessStatisticsPython::getWrapped()
{
  StoreObjPtr<ProcessStatistics> stats("", DataStore::c_Persistent);
  if (!stats) {
    B2ERROR("ProcessStatistics data object is not available, you either disabled statistics with --no-stats or didn't run process(path) yet.");
    return nullptr;
  }
  return &(*stats);
}

string ProcessStatisticsPython::getStatisticsString(ModuleStatistics::EStatisticCounters mode,
                                                    const std::vector<ModuleStatistics>* modules)
{
  if (!getWrapped())
    return "";
  return getWrapped()->getStatisticsString(mode, modules);
}

string ProcessStatisticsPython::getModuleStatistics(const boost::python::list& modulesPyList,
                                                    ModuleStatistics::EStatisticCounters mode)
{
  if (!getWrapped())
    return "";

  std::vector<ModuleStatistics> moduleStats;
  auto modules = PyObjConvUtils::convertPythonObject(modulesPyList, std::vector<ModulePtr>());
  for (ModulePtr ptr : modules) {
    ModuleStatistics& stats = getWrapped()->getStatistics(ptr.get());
    //Name could be empty if module has never been called
    if (stats.getName().empty()) stats.setName(ptr->getName());
    moduleStats.push_back(stats);
  }
  return getStatisticsString(mode, &moduleStats);
}

boost::python::list ProcessStatisticsPython::getAll()
{
  boost::python::list result;
  if (!getWrapped())
    return result;
  for (auto& module : getWrapped()->getAll()) {
    result.append(module);
  }
  return result;
}

const ModuleStatistics* ProcessStatisticsPython::get(ModulePtr module)
{
  if (!getWrapped())
    return nullptr;
  return &getWrapped()->getStatistics(module.get());
}

const ModuleStatistics* ProcessStatisticsPython::getGlobal()
{
  if (!getWrapped())
    return nullptr;
  return &getWrapped()->getGlobal();

}
void ProcessStatisticsPython::clear()
{
  if (!getWrapped())
    return;
  getWrapped()->clear();
}


#if !defined(__GNUG__) || defined(__ICC)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
//used to make python aware of default arguments
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(stats_timeSum_overloads, getTimeSum, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(stats_timeMean_overloads, getTimeMean, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(stats_timeStddev_overloads, getTimeStddev, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(stats_memorySum_overloads, getMemorySum, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(stats_memoryMean_overloads, getMemoryMean, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(stats_memoryStddev_overloads, getMemoryStddev, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(stats_timeMemoryCorr_overloads, getTimeMemoryCorrelation, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(stats_calls_overloads, getCalls, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getStatistics_overloads, getStatisticsString, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getModuleStatistics_overloads, getModuleStatistics, 1, 2)
#if !defined(__GNUG__) || defined(__ICC)
#else
#pragma GCC diagnostic pop
#endif

void ProcessStatisticsPython::exposePythonAPI()
{
  //Reference to global scope
  scope global;

  docstring_options options(true, true, false); //userdef, py sigs, c++ sigs

  //Wrap ProcessStatisticsPython as non-copy and non-instantiable in python
  class_<ProcessStatisticsPython, boost::noncopyable> stats("ProcessStatisticsPython",
                                                            R"(Interface for retrieving statistics about modules at runtime or after `basf2.process()` returns, through a global instance ``basf2.statistics``.

Statistics for event() calls are available as a string representation of the object:

>>> from basf2 import statistics
>>> print(statistics)
=================================================================================
Name                  |      Calls | VMemory(MB) |    Time(s) |     Time(ms)/Call
=================================================================================
RootInput             |        101 |          0 |       0.01 |    0.05 +-   0.02
RootOutput            |        100 |          0 |       0.02 |    0.20 +-   0.87
ProgressBar           |        100 |          0 |       0.00 |    0.00 +-   0.00
=================================================================================
Total                 |        101 |          0 |       0.03 |    0.26 +-   0.86
=================================================================================

This provides information on the number of calls, elapsed time, and the average
difference in virtual memory before and after the event() call.
Note that the module responsible for reading (or generating) events usually
has one additional event() call which is used to determine whether event
processing should stop.

Information on other calls like initialize(), terminate(), etc. are also available:

>>> print(statistics(statistics.INIT))
>>> print(statistics(statistics.BEGIN_RUN))
>>> print(statistics(statistics.END_RUN))
>>> print(statistics(statistics.TERM))
)", no_init);

  stats
  .def("set_name", &ProcessStatisticsPython::setModuleName, R"(Set name for module in statistics.

Normally, all modules get assigned their default name which is
used to register them. If multiple instances of the same module
are present at the same time, this can be used to distuingish
between them.)")
  .def("__str__", &ProcessStatisticsPython::getStatisticsString, getStatistics_overloads())
  .def("__call__", &ProcessStatisticsPython::getStatisticsString, getStatistics_overloads())
  .def("__call__", &ProcessStatisticsPython::getModuleStatistics, getModuleStatistics_overloads())
  .def("get", &ProcessStatisticsPython::get, return_value_policy<reference_existing_object>(),
       "Get `ModuleStatistics` for given Module.")
  .def("get_global", &ProcessStatisticsPython::getGlobal, return_value_policy<reference_existing_object>(),
       "Get global `ModuleStatistics` containing total elapsed time etc.")
  .def("clear", &ProcessStatisticsPython::clear, "Clear collected statistics but keep names of modules")
  .def_readonly("modules", &ProcessStatisticsPython::getAll, "List of all `ModuleStatistics` objects.")
  ;

  //Set scope to current class
  scope statistics = stats;

  //Define enum for all the counter types in scope of class
  enum_<ModuleStatistics::EStatisticCounters>("EStatisticCounters", "Select type of statistics (corresponds to Module functions)")
  .value("INIT", ModuleStatistics::c_Init)
  .value("BEGIN_RUN", ModuleStatistics::c_BeginRun)
  .value("EVENT", ModuleStatistics::c_Event)
  .value("END_RUN", ModuleStatistics::c_EndRun)
  .value("TERM", ModuleStatistics::c_Term)
  .value("TOTAL", ModuleStatistics::c_Total)
  ;

  //No we can add shorthand attributes for easier access
  stats.attr("INIT") = ModuleStatistics::c_Init;
  stats.attr("BEGIN_RUN") = ModuleStatistics::c_BeginRun;
  stats.attr("EVENT") = ModuleStatistics::c_Event;
  stats.attr("END_RUN") = ModuleStatistics::c_EndRun;
  stats.attr("TERM") = ModuleStatistics::c_Term;
  stats.attr("TOTAL") = ModuleStatistics::c_Total;

  //Wrap statistics class
  class_<ModuleStatistics>("ModuleStatistics", "Statistics for a single module.")
  .add_property("name", make_function(&ModuleStatistics::getName, return_value_policy<copy_const_reference>()),
                &ModuleStatistics::setName)
  .add_property("index", &ModuleStatistics::getIndex, &ModuleStatistics::setIndex)
  .def("time_sum", &ModuleStatistics::getTimeSum, stats_timeSum_overloads())
  .def("time_mean", &ModuleStatistics::getTimeMean, stats_timeMean_overloads())
  .def("time_stddev", &ModuleStatistics::getTimeStddev, stats_timeStddev_overloads())
  .def("memory_sum", &ModuleStatistics::getMemorySum, stats_memorySum_overloads())
  .def("memory_mean", &ModuleStatistics::getMemoryMean, stats_memoryMean_overloads())
  .def("memory_stddev", &ModuleStatistics::getMemoryStddev, stats_memoryStddev_overloads())
  .def("time_memory_corr", &ModuleStatistics::getTimeMemoryCorrelation, stats_timeMemoryCorr_overloads())
  .def("calls", &ModuleStatistics::getCalls, stats_calls_overloads())
  ;

  //Expose ProcessStatisticsPython instance as "statistics" object in pybasf2 module
  ProcessStatisticsPython& instance = getInstance();
  global.attr("statistics") = object(ptr(&instance));
}
