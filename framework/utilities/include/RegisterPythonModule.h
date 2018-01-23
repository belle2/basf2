/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  /** Proxy class to register python modules (the things you can 'import').
   *
   * \sa REGISTER_PYTHON_MODULE
   * \sa REGISTER_PYTHON_MODULE_AUTOIMPORT
   */
  class BoostPythonModuleProxy {
  public:
    /** Don't construct this object yourself, use the REGISTER_PYTHON_MODULE macro instead. */
    BoostPythonModuleProxy(const char* name, PyObject * (*initFunc)(), bool auto_import = false);
  };

  /**
   * Register a python module to make available when loading the library.
   *
   * Use it by defining your module via BOOST_PYTHON_MODULE(name), then call
   * REGISTER_PYTHON_MODULE(name) with the same name.
   *
   * After the library has been loaded, you can use 'import name' to import the
   * defined objects.
   */
#define REGISTER_PYTHON_MODULE(moduleName) Belle2::BoostPythonModuleProxy boostPythonModuleProxy##moduleName(#moduleName, PyInit_##moduleName);

  /**
   * Identical to REGISTER_PYTHON_MODULE(), but will also
   * import the module into the global namespace after loading it.
   *
   * This might be useful to allow extra definitions for individual basf2 modules,
   * consider for example:
   *
   * \code

     //in MyTestModule.h (preferably as a member of your Module class):
     enum class Shape {
       Sphere,
       Cube
     };

     //in MyTestModule.cc
     #include <boost/python.hpp>
     #include <framework/utilities/RegisterPythonModule.h>
     //...
     using namespace boost::python;

     BOOST_PYTHON_MODULE(MyTest)
     {
       //export enum to python, e.g. Shape::Sphere becomes Shape.Sphere
       enum_<Shape>("Shape")
         .value("Sphere", Shape::Sphere)
         .value("Cube", Shape::Cube)
         ;
     }
     REGISTER_PYTHON_MODULE_AUTOIMPORT(MyTest)

     \endcode
   *
   * In a steering file, the type MyTest.Sphere is available immediately after
   * register_module('MyTest').
   */
#define REGISTER_PYTHON_MODULE_AUTOIMPORT(moduleName) Belle2::BoostPythonModuleProxy boostPythonModuleProxy##moduleName(#moduleName, PyInit_##moduleName, true);
}
