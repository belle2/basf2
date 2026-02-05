/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/utilities/RegisterPythonModule.h>
#include <framework/logging/Logger.h>

#include <Python.h>

#include <string>

using namespace Belle2;

BoostPythonModuleProxy::BoostPythonModuleProxy(const char* name, PyObject * (*initFunc)(), bool auto_import)
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
