/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
