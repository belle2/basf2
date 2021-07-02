/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
