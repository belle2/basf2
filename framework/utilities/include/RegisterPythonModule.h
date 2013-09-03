/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef REGISTERPYTHONMODULE_H
#define REGISTERPYTHONMODULE_H

#include <framework/logging/Logger.h>

namespace Belle2 {
  /** Proxy class to register python modules (i.e. the things you can 'import')
   *
   * Use it by defining your module via BOOST_PYTHON_MODULE(name), then call
   * REGISTER_PYTHON_MODULE(name) with the same name.
   *
   * After the library has been loaded, you can use 'import name' to import the
   * defined objects.
   * This might be useful to allow extra definitions for individual basf2 modules,
   * which can be imported after registering the module.
   */
  class BoostPythonModuleProxy {
  public:
    /** Don't construct this object yourself, use the REGISTER_PYTHON_MODULE macro instead. */
    BoostPythonModuleProxy(const char* name, void (*initFunc)()) {
      if (PyImport_AppendInittab(const_cast<char*>(name), initFunc) == -1) {
        B2FATAL("BoostPythonModuleProxy for " << name << " failed.");
      }
    }
  };

#define REGISTER_PYTHON_MODULE(moduleName) BoostPythonModuleProxy boostPythonModuleProxy##moduleName(#moduleName, init##moduleName);
}
#endif
