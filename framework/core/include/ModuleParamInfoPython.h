/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <boost/python/list.hpp>

#include <string>

namespace Belle2 {
  /**
   * Class to store basic information about a parameter.
   * This class is used in the Python API to provide the user with information about a parameter.
   */
  class ModuleParamInfoPython {

  public:

    std::string m_name;                  /**< The name of the parameter. */
    std::string m_typeName;              /**< The name of the type of the parameter. */
    boost::python::list m_defaultValues; /**< The default values of the parameter as a python list. */
    boost::python::list m_values;        /**< The values of the parameter as a python list. */
    std::string m_description;           /**< The description of the parameter. */
    bool m_forceInSteering;              /**< If true the parameter has to be set by the user in the steering file. */
    bool m_setInSteering;                /**< True if the parameter was set in the steering file. */


    /**
     * Exposes methods of the ModuleParam class to Python.
     */
    static void exposePythonAPI();
  };
}
