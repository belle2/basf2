/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/ProcessStatistics.h>
#include <framework/gearbox/Unit.h>

#include <boost/python.hpp>
#include <boost/format.hpp>
#include <sstream>

using namespace std;
using namespace boost::python;

namespace Belle2 {

  ProcessStatistics& ProcessStatistics::getInstance()
  {
    static ProcessStatistics instance;
    return instance;
  }

  string ProcessStatistics::getStatistics(ModuleStatistics::EStatisticCounters mode, StatisticsMap* modules)
  {
    if (!modules) modules = &m_modules;
    stringstream out;
    out << boost::format("%|80T=|\n");
    boost::format outputheader("%s %|22t|| %10s | %10s | %10s | %17s\n");
    boost::format output("%s %|22t|| %10.0f | %10.0f | %10.2f | %7.2f +-%7.2f\n");
    out << outputheader % "Name" % "Calls" % "Memory(MB)" % "Time(s)" % "Time(ms)/Call";
    out << boost::format("%|80T-|\n");

    const int numModules = modules->size();
    for (int iModule = 0; iModule < numModules; iModule++) {
      //find Module with ID = iModule, to list them by initialisation
      for (auto & it : *modules) {
        const ModuleStatistics& stats = it.second;
        if (stats.getIndex() != iModule)
          continue;

        out << output
            % stats.getName()
            % stats.getCalls(mode)
            % (stats.getMemorySum(mode) / 1024)
            % (stats.getTimeSum(mode) / Unit::s)
            % (stats.getTimeMean(mode) / Unit::ms)
            % (stats.getTimeStddev(mode) / Unit::ms);
      }
    }

    out << boost::format("%|80T-|\n");
    out << output
        % "Total"
        % m_global.getCalls(mode)
        % (m_global.getMemorySum(mode) / 1024)
        % (m_global.getTimeSum(mode) / Unit::s)
        % (m_global.getTimeMean(mode) / Unit::ms)
        % (m_global.getTimeStddev(mode) / Unit::ms);
    out << boost::format("%|80T=|\n");
    return out.str();
  }

  string ProcessStatistics::getModuleStatistics(const boost::python::list& pyList, ModuleStatistics::EStatisticCounters mode)
  {
    StatisticsMap modules;
    int nList = boost::python::len(pyList);
    for (int i = 0; i < nList; ++i) {
      extract<ModulePtr> checkValue(pyList[i]);
      if (checkValue.check()) {
        ModulePtr ptr = checkValue;
        ModuleStatistics& stats = m_modules[ptr.get()];
        //Name could be empty if module has never been called
        if (stats.getName().empty()) stats.setName(ptr->getName());
        modules[ptr.get()] = stats;
      }
    }
    return getStatistics(mode, &modules);
  }

  boost::python::list ProcessStatistics::getAll()
  {
    boost::python::list result;
    for (StatisticsMap::value_type & module : m_modules) {
      result.append(module.second);
    }
    return result;
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
  BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getStatistics_overloads, getStatistics, 0, 1)
  BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getModuleStatistics_overloads, getModuleStatistics, 1, 2)

  void ProcessStatistics::exposePythonAPI()
  {
    //Reference to global scope
    scope global;

    //Wrap ProcessStatistics as non-copy and non-instantiable in python
    class_<ProcessStatistics, boost::noncopyable> stats("ProcessStatistics", no_init);

    stats
//    .def("set_name", &ProcessStatistics::setModuleName)
    .def("__str__", &ProcessStatistics::getStatistics, getStatistics_overloads())
    .def("__call__", &ProcessStatistics::getStatistics, getStatistics_overloads())
    .def("__call__", &ProcessStatistics::getModuleStatistics, getModuleStatistics_overloads())
    .def("get", &ProcessStatistics::get, return_value_policy<reference_existing_object>())
    .def("clear", &ProcessStatistics::clear)
    .def_readonly("framework", &ProcessStatistics::m_global)
    .def_readonly("modules", &ProcessStatistics::getAll)
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
    .def("time_sum", &ModuleStatistics::getTimeSum, stats_timeSum_overloads())
    .def("time_mean", &ModuleStatistics::getTimeMean, stats_timeMean_overloads())
    .def("time_stddev", &ModuleStatistics::getTimeStddev, stats_timeStddev_overloads())
    .def("memory_sum", &ModuleStatistics::getMemorySum, stats_memorySum_overloads())
    .def("memory_mean", &ModuleStatistics::getMemoryMean, stats_memoryMean_overloads())
    .def("memory_stddev", &ModuleStatistics::getMemoryStddev, stats_memoryStddev_overloads())
    .def("time_memory_corr", &ModuleStatistics::getTimeMemoryCorrelation, stats_timeMemoryCorr_overloads())
    .def("calls", &ModuleStatistics::getCalls, stats_calls_overloads())
    ;

    //Expose ProcessStatistics instance as "statistics" object in pybasf2 module
    ProcessStatistics& instance = getInstance();
    global.attr("statistics") = object(ptr(&instance));
  }

} //end namespace Belle2
