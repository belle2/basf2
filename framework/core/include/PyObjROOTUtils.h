/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <boost/python/object.hpp>
#include <TPython.h>

namespace Belle2 {
  /** Create a python wrapped copy from a class instance which has a ROOT dictionary.
   * This piece of dark magic creates a python object referencing a ROOT object
   * the same way as you would see it in the ROOT python module.
   *
   * It will create a copy of the object using the copy constructor which is then
   * owned by python.
   */
  template<class T>
  boost::python::object createROOTObjectPyCopy(const T& instance)
  {
    const char* classname = instance.IsA()->GetName();
    void* addr = new T(instance);
    PyObject* obj = TPython::CPPInstance_FromVoidPtr(addr, classname, true);
    return boost::python::object(boost::python::handle<>(obj));
  }
}
