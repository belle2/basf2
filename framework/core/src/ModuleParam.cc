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

#include <framework/core/ModuleParam.h>

using namespace std;
using namespace Belle2;
using namespace boost::python;

void ModuleParamInfoPython::exposePythonAPI()
{
  //Python class definition
  boost::python::class_<ModuleParamInfoPython>("ModuleParamInfo")
  .def_readonly("name", &ModuleParamInfoPython::m_name)
  .def_readonly("type", &ModuleParamInfoPython::m_typeName)
  .def_readonly("default", &ModuleParamInfoPython::m_defaultValues)
  .def_readonly("values", &ModuleParamInfoPython::m_values)
  .def_readonly("description", &ModuleParamInfoPython::m_description)
  .def_readonly("forceInSteering", &ModuleParamInfoPython::m_forceInSteering)
  .def_readonly("setInSteering", &ModuleParamInfoPython::m_setInSteering)
  ;
}
