/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python.hpp>

#include <framework/utilities/RegisterPythonModule.h>
#include <framework/logging/Logger.h>

#include <string>

using namespace Belle2;

BoostPythonModuleProxy::BoostPythonModuleProxy(const char* name, void (*initFunc)(), bool auto_import)
{
  if (PyImport_AppendInittab(const_cast<char*>(name), initFunc) == -1) {
    B2FATAL("REGISTER_PYTHON_MODULE(" << name << ") failed.");
  }

  if (auto_import) {
    if (!Py_IsInitialized()) {
      B2FATAL("REGISTER_PYTHON_MODULE_AUTOIMPORT(" << name << "): Python is not initialized!");
    }

    std::string importString = "import ";
    importString += name;
    if (PyRun_SimpleString(importString.c_str()) == -1) {
      B2FATAL("'import " << name << "' failed.");
    }
  }
}
