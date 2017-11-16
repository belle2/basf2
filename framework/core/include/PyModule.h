/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <boost/python/call_method.hpp>
#include <framework/core/Module.h>

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
      m_self(p)
    {
      setName(p->ob_type->tp_name);
      setType("PyModule");
    }

    /** copy constructor */
    PyModule(PyObject* p, const Module& m):
      Module(m),
      m_self(p)
    {
      setName(p->ob_type->tp_name);
      setType("PyModule");
    }


    /* reimplement all virtual functions of base class and call the corresponding method in the python class
    *  note that all methods must be registered in Module::exposePythonAPI()
    */

    virtual void initialize() override final
    {
      boost::python::call_method<void>(m_self, "initialize");
    }

    virtual void beginRun() override final
    {
      boost::python::call_method<void>(m_self, "beginRun");
    }

    virtual void event() override final
    {
      boost::python::call_method<void>(m_self, "event");
    }

    virtual void endRun() override final
    {
      boost::python::call_method<void>(m_self, "endRun");
    }

    virtual void terminate() override final
    {
      boost::python::call_method<void>(m_self, "terminate");
    }

  private:
    //@{
    /** default implementation used when Python module doesn't supply its own */
    virtual void def_initialize() override final
    {
      // Call empty base implementation if initialize() is not overriden from Python.
      Module::initialize();
    }

    virtual void def_beginRun() override final
    {
      // Call empty base implementation if beginRun() is not overriden from Python.
      Module::beginRun();
    }

    virtual void def_event() override final
    {
      // Call empty base implementation if event() is not overriden from Python.
      Module::event();
    }

    virtual void def_endRun() override final
    {
      // Call empty base implementation if endRun() is not overriden from Python.
      Module::endRun();
    }

    virtual void def_terminate() override final
    {
      // Call empty base implementation if terminate() is not overriden from Python.
      Module::terminate();
    }
    //@}

    PyObject* m_self; /**< the actual python module */
  };
}
