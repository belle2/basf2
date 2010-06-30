/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/kbasf2/kbasf2pymodule.h>

using namespace Belle2;
using namespace boost::python;


//-----------------------------------
//   Define the kbasf2 python module
//-----------------------------------
BOOST_PYTHON_MODULE(kbasf2)
{
  Module::exposePythonAPI();
  Path::exposePythonAPI();
  Framework::exposePythonAPI();
  ModuleParamList::exposePythonAPI();
  Gearbox::exposePythonAPI();
}


//! Creates the basf2 python module.
void Kbasf2PyModule::embeddPythonModule() throw(FwExcPythonModuleNotEmbedded)
{
  if (PyImport_AppendInittab(const_cast<char*>("kbasf2"), initkbasf2) == -1) {
    throw FwExcPythonModuleNotEmbedded();
  }
}

