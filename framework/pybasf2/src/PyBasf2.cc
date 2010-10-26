/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/pybasf2/PyBasf2.h>

#include <boost/python.hpp>

#include <framework/core/Module.h>
#include <framework/core/Path.h>
#include <framework/core/Framework.h>
#include <framework/pcore/pFramework.h>
#include <framework/core/ModuleParam.h>

using namespace Belle2;
using namespace boost::python;


//-----------------------------------
//   Define the pybasf2 python module
//-----------------------------------
BOOST_PYTHON_MODULE(pybasf2)
{
  Module::exposePythonAPI();
  Path::exposePythonAPI();
  Framework::exposePythonAPI();
  pFramework::exposePythonAPI();
  ModuleParamInfoPython::exposePythonAPI();
}


//! Creates the basf2 Python module.
void PyBasf2::embedPythonModule() throw(PythonModuleNotEmbeddedError)
{
  if (PyImport_AppendInittab(const_cast<char*>("pybasf2"), initpybasf2) == -1) {
    throw PythonModuleNotEmbeddedError();
  }
}

