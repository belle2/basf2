/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/ModuleStatistics.h>
#include <sstream>
#include <boost/format.hpp>
#include <boost/python.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/mpl/vector.hpp>

using namespace std;
using namespace boost::python;

namespace Belle2 {

  /** return reference to Instance */
  ModuleStatistics& ModuleStatistics::getInstance()
  {
    static ModuleStatistics instance;
    return instance;
  }

  string ModuleStatistics::getStatistics(ECounters mode, StatisticsMap *modules)
  {
    if (!modules) modules = &m_modules;
    stringstream out;
    out << boost::format("%|79T=|\n");
    boost::format output("%s %|32t|| %10d | %14.3f | %14.3f\n");
    out << output % "Name" % "Calls" % "Time(s)" % "Time(ms)/Call";
    out << boost::format("%|79T-|\n");
    for (StatisticsMap::const_iterator it = modules->begin(); it != modules->end(); ++it) {
      const Statistics &stats = it->second;
      out << output
      % stats.getName()
      % stats.getCalls(mode)
      % stats.getTime(mode)
      % (stats.getCalls(mode) > 0 ? (1e3*stats.getTime(mode) / stats.getCalls(mode)) : 0);
    }
    out << boost::format("%|79T-|\n");
    out << output
    % "Total"
    % m_global.getCalls(mode)
    % m_global.getTime(mode)
    % (m_global.getCalls(mode) > 0 ? (1e3*m_global.getTime(mode) / m_global.getCalls(mode)) : 0);
    out << boost::format("%|79T=|\n");
    return out.str();
  }

  /** print event statistics for given modules to stream */
  string ModuleStatistics::getModuleStatistics(const boost::python::list& pyList, ECounters mode)
  {
    StatisticsMap modules;
    int nList = boost::python::len(pyList);
    for (int i = 0; i < nList; ++i) {
      extract<ModulePtr> checkValue(pyList[i]);
      if (checkValue.check()) {
        ModulePtr ptr = checkValue;
        Statistics &stats = m_modules[ptr.get()];
        //Name could be empty if module has never been called
        if (stats.m_name.empty()) stats.m_name = ptr->getName();
        modules[ptr.get()] = stats;
      }
    }
    return getStatistics(mode, &modules);
  }

  /** get list of all statistic objects */
  boost::python::list ModuleStatistics::getAll()
  {
    boost::python::list result;
    BOOST_FOREACH(StatisticsMap::value_type &module, m_modules) {
      result.append(module.second);
    }
    return result;
  }

  /** clear counters, keep names */
  void ModuleStatistics::clear()
  {
    BOOST_FOREACH(StatisticsMap::value_type &module, m_modules) {
      module.second.clear();
    }
    m_global.clear();
  }

  BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(stats_time_overloads, getTime, 0, 1);
  BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(stats_calls_overloads, getCalls, 0, 1);
  BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getStatistics_overloads, getStatistics, 0, 1);
  BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getModuleStatistics_overloads, getModuleStatistics, 1, 2);

  /** Expose python API */
  void ModuleStatistics::exposePythonAPI()
  {
    //Reference to global scope
    scope global;

    //Wrap ModuleStatistics as non-copy and non-instantiable in python
    class_<ModuleStatistics, boost::noncopyable> stats("ModuleStatistics", no_init);

    stats
    .def("set_name", &ModuleStatistics::setModuleName)
    .def("__str__", &ModuleStatistics::getStatistics, getStatistics_overloads())
    .def("__call__", &ModuleStatistics::getStatistics, getStatistics_overloads())
    .def("__call__", &ModuleStatistics::getModuleStatistics, getModuleStatistics_overloads())
    .def("get", &ModuleStatistics::get, return_value_policy<reference_existing_object>())
    .def("clear", &ModuleStatistics::clear)
    .def_readonly("framework", &ModuleStatistics::m_global)
    .def_readonly("modules", &ModuleStatistics::getAll)
    ;

    //Set scope to current class
    scope statistics = stats;

    //Define enum for all the counter types in scope of class
    enum_<ECounters>("ECounters")
    .value("INIT", c_Init)
    .value("BEGIN_RUN", c_BeginRun)
    .value("EVENT", c_Event)
    .value("END_RUN", c_EndRun)
    .value("TERM", c_Term)
    .value("TOTAL", c_Total)
    ;

    //No we can add shorthand attributes for easier access
    stats.attr("INIT") = c_Init;
    stats.attr("BEGIN_RUN") = c_BeginRun;
    stats.attr("EVENT") = c_Event;
    stats.attr("END_RUN") = c_EndRun;
    stats.attr("TERM") = c_Term;
    stats.attr("TOTAL") = c_Total;

    //Wrap statistics class
    class_<Statistics>("Statistics")
    .def_readonly("name", &Statistics::getName)
    .def("time", &Statistics::getTime, stats_time_overloads())
    .def("calls", &Statistics::getCalls, stats_calls_overloads())
    ;

    //Expose ModuleStatistics instance as "statistics" object in pybasf2 module
    ModuleStatistics &instance = getInstance();
    global.attr("statistics") = object(ptr(&instance));
  }

} //end namespace Belle2
