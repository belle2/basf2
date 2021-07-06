/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <boost/python.hpp>
#include <framework/pybasf2/ProcessStatisticsPython.h>
#include <framework/core/Module.h>
#include <framework/core/Environment.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/core/PyObjConvUtils.h>

using namespace Belle2;
using namespace std;
using namespace boost::python;

ProcessStatistics* ProcessStatisticsPython::getWrapped()
{
  StoreObjPtr<ProcessStatistics> stats("", DataStore::c_Persistent);
  if (!stats) {
    if (!Environment::Instance().getDryRun()) {
      B2ERROR("ProcessStatistics data object is not available, you either disabled statistics with --no-stats or didn't run process(path) yet.");
    }
    return nullptr;
  }
  return &(*stats);
}

string ProcessStatisticsPython::getStatisticsString()
{
  if (!getWrapped())
    return "";
  return getWrapped()->getStatisticsString(m_type, m_modules.empty() ? nullptr : &m_modules);
}

string ProcessStatisticsPython::getStatisticsStringHTML()
{
  if (!getWrapped())
    return "";
  return getWrapped()->getStatisticsString(m_type, m_modules.empty() ? nullptr : &m_modules, true);
}

ProcessStatisticsPython ProcessStatisticsPython::getModuleStatistics(ModuleStatistics::EStatisticCounters type,
    const boost::python::list& modulesPyList)
{
  if (!getWrapped())
    return ProcessStatisticsPython();

  std::vector<ModuleStatistics> moduleStats;
  auto modules = PyObjConvUtils::convertPythonObject(modulesPyList, std::vector<ModulePtr>());
  for (const ModulePtr& ptr : modules) {
    ModuleStatistics& stats = getWrapped()->getStatistics(ptr.get());
    //Name could be empty if module has never been called
    if (stats.getName().empty()) stats.setName(ptr->getName());
    moduleStats.push_back(stats);
  }
  return ProcessStatisticsPython(type, moduleStats);
}

boost::python::list ProcessStatisticsPython::getAll()
{
  boost::python::list result;
  if (!getWrapped())
    return result;
  for (auto& module : (m_modules.empty()) ? getWrapped()->getAll() : m_modules) {
    result.append(module);
  }
  return result;
}

const ModuleStatistics* ProcessStatisticsPython::get(const ModulePtr& module)
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


void ProcessStatisticsPython::exposePythonAPI()
{
  // to avoid confusion between std::arg and boost::python::arg we want a shorthand namespace as well
  namespace bp = boost::python;

  //Reference to global scope
  scope global;

  docstring_options options(true, true, false); //userdef, py sigs, c++ sigs

  //Wrap ProcessStatisticsPython as non-copy and non-instantiable in python
  class_<ProcessStatisticsPython> stats("ProcessStatistics", R"DOCSTRING(
Interface for retrieving statistics about module execution at runtime or after
:py:func:`basf2.process()` returns. Should be accessed through a global instance `basf2.statistics`.

Statistics for `event() <Module.event()>` calls are available as a string representation of the object:

>>> from basf2 import statistics
>>> print(statistics)
=================================================================================
Name                  |      Calls | Memory(MB) |    Time(s) |     Time(ms)/Call
=================================================================================
RootInput             |        101 |          0 |       0.01 |    0.05 +-   0.02
RootOutput            |        100 |          0 |       0.02 |    0.20 +-   0.87
ProgressBar           |        100 |          0 |       0.00 |    0.00 +-   0.00
=================================================================================
Total                 |        101 |          0 |       0.03 |    0.26 +-   0.86
=================================================================================

This provides information on the number of calls, elapsed time, and the average
difference in resident memory before and after the `event() <Module.event>` call.

.. note::

    The module responsible for reading (or generating) events usually has one
    additional event() call which is used to determine whether event processing
    should stop.

.. warning::

    Memory consumption is reporting the difference in memory usage as reported
    by the kernel before and after the call. This is not the maximum memory the
    module has consumed. Negative values indicate that this module has freed
    momemory which was allocated in other modules or function calls.

Information on other calls like `initialize() <Module.initialize>`,
`terminate() <Module.terminate>`, etc. are also available through the different
counters defined in `StatisticCounters`:

>>> print(statistics(statistics.INIT))
>>> print(statistics(statistics.BEGIN_RUN))
>>> print(statistics(statistics.END_RUN))
>>> print(statistics(statistics.TERM))
)DOCSTRING", no_init);

  stats
  .def("get", &ProcessStatisticsPython::get, return_value_policy<reference_existing_object>(), bp::arg("module"),
       "Get `ModuleStatistics` for given Module.")
  .def("get_global", &ProcessStatisticsPython::getGlobal, return_value_policy<reference_existing_object>(),
       "Get global `ModuleStatistics` containing total elapsed time etc.")
  .def("clear", &ProcessStatisticsPython::clear, "Clear collected statistics but keep names of modules")
  .def_readonly("modules", &ProcessStatisticsPython::getAll, "List of all `ModuleStatistics` objects.")
  ;

  //Set scope to current class
  scope statistics{stats};
  //Define enum for all the counter types in scope of class
  enum_<ModuleStatistics::EStatisticCounters>("StatisticCounters", R"DOCSTRING(
Available types of statistic counters (corresponds to Module functions)

.. attribute:: INIT

Time spent or memory used in the `initialize() <Module.initialize>` function

.. attribute:: BEGIN_RUN

Time spent or memory used in the `beginRun() <Module.beginRun>` function

.. attribute:: EVENT

Time spent or memory used in the `event() <Module.event>` function

.. attribute:: END_RUN

Time spent or memory used in the `endRun() <Module.endRun>` function

.. attribute:: TERM

Time spent or memory used in the `terminate() <Module.terminate>` function

.. attribute:: TOTAL

Time spent or memory used in any module function. This is the sum of all of the above.

)DOCSTRING")
  .value("INIT", ModuleStatistics::c_Init)
  .value("BEGIN_RUN", ModuleStatistics::c_BeginRun)
  .value("EVENT", ModuleStatistics::c_Event)
  .value("END_RUN", ModuleStatistics::c_EndRun)
  .value("TERM", ModuleStatistics::c_Term)
  .value("TOTAL", ModuleStatistics::c_Total)
  .export_values()
  ;

  {
    // the overloaded __str__ and __call__ give very confusing signatures so hand-craft doc string.
  docstring_options custom_options(true, false, false); //userdef, py sigs, c++ sigs
  stats
  .def("__str__", &ProcessStatisticsPython::getStatisticsString,
       "Return the event statistics as a string in a human readable form")
  .def("_repr_html_", &ProcessStatisticsPython::getStatisticsStringHTML,
       "Return an html represenation of the statistics (used by ipython/jupyter)")
  .def("__call__", &ProcessStatisticsPython::getModuleStatistics, (bp::arg("counter") = ModuleStatistics::EStatisticCounters::c_Event, bp::arg("modules") = boost::python::list()),
       R"DOCSTRING(__call__(counter=StatisticCounters.EVENT, modules=None)

Calling the statistics object directly like a function will return a string
with the execution statistics in human readable form.

Parameters:
    counter (StatisticCounters): Which counter to use
    modules (list[Module]): A list of modules to include in the returned string.
        If omitted the statistics for all modules will be included.

* print the `beginRun() <Module.beginRun>` statistics for all modules:

  >>> print(statistics(statistics.BEGIN_RUN))

* print the total execution times and memory consumption but only for the
  modules ``module1`` and ``module2``

  >>> print(statistics(statistics.TOTAL, [module1, module2]))

* print the event statistics (default) for only two modules

  >>> print(statistics(modules=[module1, module2]))
)DOCSTRING")
  ;
  }

  //Wrap statistics class. The default boost python docstring signature is way
  //to noisy for these simple getters so this time we do it ourselves ...
  docstring_options new_options(true, false, false); //userdef, py sigs, c++ sigs
  class_<ModuleStatistics>("ModuleStatistics", "Execution statistics for a single module. "
                           "All member functions take exactly one argument to select which "
                           "counter to query which defaults to `StatisticCounters.TOTAL` if omitted.")
  .add_property("name", make_function(&ModuleStatistics::getName, return_value_policy<copy_const_reference>()),
                "property to get the name of the module to be displayed in the statistics")
  .def("time_sum", &ModuleStatistics::getTimeSum, bp::arg("counter") = ModuleStatistics::c_Total,
       "time_sum(counter=StatisticCounters.TOTAL)\nReturn the sum of all execution times")
  .def("time_mean", &ModuleStatistics::getTimeMean, bp::arg("counter") = ModuleStatistics::c_Total,
       "time_mean(counter=StatisticCounters.TOTAL)\nReturn the mean of all execution times")
  .def("time_stddev", &ModuleStatistics::getTimeStddev, bp::arg("counter") = ModuleStatistics::c_Total,
       "time_stddev(counter=StatisticCounters.TOTAL)\nReturn the standard deviation of all execution times")
  .def("memory_sum", &ModuleStatistics::getMemorySum, bp::arg("counter") = ModuleStatistics::c_Total,
       "memory_sum(counter=StatisticCounters.TOTAL)\nReturn the sum of the total memory usage")
  .def("memory_mean", &ModuleStatistics::getMemoryMean, bp::arg("counter") = ModuleStatistics::c_Total,
       "memory_mean(counter=StatisticCounters.TOTAL)\nReturn the mean of the memory usage")
  .def("memory_stddev", &ModuleStatistics::getMemoryStddev, bp::arg("counter") = ModuleStatistics::c_Total,
       "memory_stddev(counter=StatisticCounters.TOTAL)\nReturn the standard deviation of the memory usage")
  .def("time_memory_corr", &ModuleStatistics::getTimeMemoryCorrelation, bp::arg("counter") = ModuleStatistics::c_Total,
       "time_memory_corr(counter=StatisticCounters.TOTAL)\nReturn the correlaction factor between time and memory consumption")
  .def("calls", &ModuleStatistics::getCalls, bp::arg("counter") = ModuleStatistics::c_Total,
       "calls(counter=StatisticCounters.TOTAL)\nReturn the total number of calls")
  ;

  //Expose ProcessStatisticsPython instance as "statistics" object in pybasf2 module
  global.attr("statistics") = object(ProcessStatisticsPython());
}
