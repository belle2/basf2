/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/utilities/EnvironmentVariables.h>
#include <boost/algorithm/string.hpp>
#include <boost/python/import.hpp>
#include <boost/python/extract.hpp>
#include <cstdlib>

namespace Belle2 {
  bool EnvironmentVariables::isSet(const std::string& name)
  {
    char* envValue = std::getenv(name.c_str());
    return envValue != nullptr;
  }

  std::string EnvironmentVariables::get(const std::string& name, const std::string fallback)
  {
    char* envValue = std::getenv(name.c_str());
    if (envValue != nullptr) {
      std::string val(envValue);
      boost::trim(val);
      return envValue;
    }
    return fallback;
  }

  std::vector<std::string> EnvironmentVariables::getList(const std::string& name, const std::vector<std::string>& fallback,
                                                         const std::string& separators)
  {
    if (!isSet(name)) return fallback;
    std::string value = get(name);
    std::vector<std::string> items;
    boost::split(items, value, boost::is_any_of(separators));
    return items;
  }

  std::string EnvironmentVariables::expand(const std::string& text)
  {
    // we could implement this ourselves ... but that seems like a waste of
    // time since there's a perfectly good implementation available.
    namespace py = boost::python;
    py::object path = py::import("os.path");
    return py::extract<std::string>(path.attr("expandvars")(text));
  }
}
