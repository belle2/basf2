/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/ModuleParam.dcl.h>
#include <framework/core/PyObjConvUtils.h>
#include <boost/python/object.hpp>
#include <string>

namespace Belle2 {
  template <class T>
  std::string ModuleParam<T>::TypeInfo()
  {
    return PyObjConvUtils::Type<T>::name();
  }

  template <class T>
  ModuleParam<T>::ModuleParam(T& paramVariable,
                              const std::string& description,
                              bool force)
    : ModuleParamBase(TypeInfo(), description, force)
    , m_paramVariable(paramVariable) {};

  template <class T>
  ModuleParam<T>::~ModuleParam() = default;

  template <class T>
  void ModuleParam<T>::setValue(T value)
  {
    m_paramVariable = value;
    m_setInSteering = true;
  }

  template <class T>
  void ModuleParam<T>::setDefaultValue(T defaultValue)
  {
    m_defaultValue = defaultValue;
    m_paramVariable = defaultValue;
    m_setInSteering = false;
  }

  template <class T>
  T& ModuleParam<T>::getValue()
  {
    return m_paramVariable;
  }

  template <class T>
  T& ModuleParam<T>::getDefaultValue()
  {
    return m_defaultValue;
  }

  template <class T>
  void ModuleParam<T>::setValueFromPythonObject(const boost::python::object& pyObject)
  {
    setValue(PyObjConvUtils::convertPythonObject(pyObject, getDefaultValue()));
  }

  template <class T>
  void ModuleParam<T>::setValueToPythonObject(boost::python::object& pyObject,
                                              bool defaultValues) const
  {
    pyObject =
      PyObjConvUtils::convertToPythonObject((defaultValues) ? m_defaultValue : m_paramVariable);
  }

  template <class T>
  void ModuleParam<T>::setValueFromParam(const ModuleParamBase& param)
  {
    const ModuleParam<T>* p = dynamic_cast<const ModuleParam<T>*>(&param);
    if (p) {
      m_defaultValue = p->m_defaultValue;
      m_paramVariable = p->m_paramVariable;
      m_setInSteering = p->m_setInSteering;
    }
  }

  template <class T>
  void ModuleParam<T>::resetValue()
  {
    m_paramVariable = m_defaultValue;
    m_setInSteering = false;
  }

} // end of Belle2 namespace
