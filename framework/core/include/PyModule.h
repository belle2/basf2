/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PYMODULE_H
#define PYMODULE_H

#include <boost/python/call_method.hpp>

#include <framework/core/Module.h>

#include <string>

namespace Belle2 {
  /** Python wrapper for Belle2::Module.
   *
   * Used internally when deriving a python class from Module to ensure that
   * reimplemented virtual functions (e.g. event()) actually get called.
   * The C++ part of the framework will only see a PyModule, which will call
   * the corresponding methods of the python module (m_self)
   *
   * See framework/examples/cdcplotmodule.py for an example module. Note that
   * the CDCPlotModule class inherits from Module, not PyModule.
   */
  class PyModule : public Module {
  public:
    /** constructor */
    PyModule(PyObject* p):
      Module(),
      m_self(p) {
      setModuleName(p->ob_type->tp_name);
      setType("PyModule");
    }

    /** copy constructor */
    PyModule(PyObject* p, const Module& m):
      Module(m),
      m_self(p) {
      setModuleName(p->ob_type->tp_name);
      setType("PyModule");
    }


    /* reimplement all virtual functions of base class and call the corresponding method in the python class
    *  note that all methods must be registered in Module::exposePythonAPI()
    */

    void initialize() { boost::python::call_method<void>(m_self, "initialize"); };
    void beginRun() { boost::python::call_method<void>(m_self, "beginRun"); };
    void event() { boost::python::call_method<void>(m_self, "event"); };
    void endRun() { boost::python::call_method<void>(m_self, "endRun"); };
    void terminate() { boost::python::call_method<void>(m_self, "terminate"); };

    //@{
    /** default implementation used when Python module doesn't supply its own */
    static void def_initialize(Module& m) { m.Module::initialize(); }
    static void def_beginRun(Module& m) { m.Module::beginRun(); }
    static void def_event(Module& m) { m.Module::event(); }
    static void def_endRun(Module& m) { m.Module::endRun(); }
    static void def_terminate(Module& m) { m.Module::terminate(); }
    //@}

  private:
    PyObject* m_self; /**< the actual python module */
  };
}
#endif
