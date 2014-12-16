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

string ProcessStatisticsPython::getStatisticsString(ModuleStatistics::EStatisticCounters mode, const std::vector<ModuleStatistics>* modules)
{
  if (!getWrapped())
    return "";
  return getWrapped()->getStatisticsString(mode, modules);
}

string ProcessStatisticsPython::getModuleStatistics(const boost::python::list& modulesPyList, ModuleStatistics::EStatisticCounters mode)
{
  if (!getWrapped())
    return "";
  std::vector<ModuleStatistics> modules;
  int nList = boost::python::len(modulesPyList);
  for (int i = 0; i < nList; ++i) {
    extract<ModulePtr> checkValue(modulesPyList[i]);
    if (checkValue.check()) {
      ModulePtr ptr = checkValue;
      ModuleStatistics& stats = getWrapped()->getStatistics(ptr.get());
      //Name could be empty if module has never been called
      if (stats.getName().empty()) stats.setName(ptr->getName());
      modules.push_back(stats);
    }
  }
  return getStatisticsString(mode, &modules);
}

boost::python::list ProcessStatisticsPython::getAll()
{
  boost::python::list result;
  if (!getWrapped())
    return result;
  for (auto & module : getWrapped()->getAll()) {
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

void ProcessStatisticsPython::exposePythonAPI()
{
  //Reference to global scope
  scope global;

  //Wrap ProcessStatisticsPython as non-copy and non-instantiable in python
  class_<ProcessStatisticsPython, boost::noncopyable> stats("ProcessStatisticsPython", no_init);

  stats
  .def("set_name", &ProcessStatisticsPython::setModuleName)
  .def("__str__", &ProcessStatisticsPython::getStatisticsString, getStatistics_overloads())
  .def("__call__", &ProcessStatisticsPython::getStatisticsString, getStatistics_overloads())
  .def("__call__", &ProcessStatisticsPython::getModuleStatistics, getModuleStatistics_overloads())
  .def("get", &ProcessStatisticsPython::get, return_value_policy<reference_existing_object>())
  .def("getGlobal", &ProcessStatisticsPython::getGlobal, return_value_policy<reference_existing_object>())
  .def("clear", &ProcessStatisticsPython::clear)
  .def_readonly("modules", &ProcessStatisticsPython::getAll)
  ;

  //Set scope to current class
  scope statistics = stats;

  //Define enum for all the counter types in scope of class
  enum_<ModuleStatistics::EStatisticCounters>("EStatisticCounters")
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
  class_<ModuleStatistics>("ModuleStatistics")
  .add_property("name", &ModuleStatistics::getName, &ModuleStatistics::setName)
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
