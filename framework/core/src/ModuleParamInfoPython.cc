/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python/class.hpp>

#include <framework/core/ModuleParamInfoPython.h>

using namespace std;
using namespace Belle2;
using namespace boost::python;

void ModuleParamInfoPython::exposePythonAPI()
{
  //Python class definition
  boost::python::class_<ModuleParamInfoPython>("ModuleParamInfo",
                                               "Read only class to provide information about a module parameter")
  .def_readonly("name", &ModuleParamInfoPython::m_name,
                "Name of the Parameter")
  .def_readonly("type", &ModuleParamInfoPython::m_typeName,
                "Type of the Parameter as string")
  .def_readonly("default", &ModuleParamInfoPython::m_defaultValues,
                "Default value of the parameter")
  .def_readonly("values", &ModuleParamInfoPython::m_values,
                "Currently set value")
  .def_readonly("description", &ModuleParamInfoPython::m_description,
                "Description of the parameter")
  .def_readonly("forceInSteering", &ModuleParamInfoPython::m_forceInSteering,
                "If True this parameter has to be set for the module to work")
  .def_readonly("setInSteering", &ModuleParamInfoPython::m_setInSteering,
                "If True this parameter has been set to a specific value")
  ;
}
