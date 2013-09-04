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
   *
   * The REGISTER_PYTHON_MODULE_AUTOIMPORT macro is identical, but will also
   * import the module into the global namespace after loading it.
   */
  class BoostPythonModuleProxy {
  public:
    /** Don't construct this object yourself, use the REGISTER_PYTHON_MODULE macro instead. */
    BoostPythonModuleProxy(const char* name, void (*initFunc)(), bool auto_import = false);
  };

#define REGISTER_PYTHON_MODULE(moduleName) Belle2::BoostPythonModuleProxy boostPythonModuleProxy##moduleName(#moduleName, init##moduleName);
#define REGISTER_PYTHON_MODULE_AUTOIMPORT(moduleName) Belle2::BoostPythonModuleProxy boostPythonModuleProxy##moduleName(#moduleName, init##moduleName, true);
}
#endif
