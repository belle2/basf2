/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParam.h>
#include <framework/logging/Logger.h>

#include <map>
#include <string>

namespace Belle2 {
  //======================================================
  //       Implementation of template based methods
  //======================================================

  template <typename T>
  void ModuleParamList::addParameter(const std::string& name,
                                     T& paramVariable,
                                     const std::string& description,
                                     const T& defaultValue)
  {
    ModuleParamPtr newParam(new ModuleParam<T>(paramVariable, description, false));

    // Check if a parameter with the given name already exists
    std::map<std::string, ModuleParamPtr>::iterator mapIter;
    mapIter = m_paramMap.find(name);

    if (mapIter == m_paramMap.end()) {
      m_paramMap.insert(std::make_pair(name, newParam));
      ModuleParam<T>* explModParam = static_cast<ModuleParam<T>*>(newParam.get());
      explModParam->setDefaultValue(defaultValue);
    } else {
      B2ERROR("A parameter with the name '" + name +
              "' already exists! The name of a module parameter must be unique within a module.");
    }
  }

  template <typename T>
  void ModuleParamList::addParameter(const std::string& name,
                                     T& paramVariable,
                                     const std::string& description)
  {
    ModuleParamPtr newParam(new ModuleParam<T>(paramVariable, description, true));

    // Check if a parameter with the given name already exists
    std::map<std::string, ModuleParamPtr>::iterator mapIter;
    mapIter = m_paramMap.find(name);

    if (mapIter == m_paramMap.end()) {
      m_paramMap.insert(std::make_pair(name, newParam));
    } else {
      B2ERROR("A parameter with the name '" + name +
              "' already exists! The name of a module parameter must be unique within a module.");
    }
  }

  template <typename T>
  void ModuleParamList::setParameter(const std::string& name, const T& value)
  {
    try {
      ModuleParam<T>& explModParam = getParameter<T>(name);
      explModParam.setValue(value);
    } catch (std::runtime_error& exc) {
      B2ERROR(exc.what());
    }
  }

  template <typename T>
  ModuleParam<T>& ModuleParamList::getParameter(const std::string& name) const
  {
    // Check if a parameter with the given name exists
    std::map<std::string, ModuleParamPtr>::const_iterator mapIter;
    mapIter = m_paramMap.find(name);

    if (mapIter != m_paramMap.end()) {
      ModuleParamPtr moduleParam = mapIter->second;

      // Check the type of the stored parameter (currently done using the type identifier string)
      if (moduleParam->getTypeInfo() == ModuleParam<T>::TypeInfo()) {
        ModuleParam<T>* explModParam = static_cast<ModuleParam<T>*>(moduleParam.get());
        return *explModParam;
      } else
        throwTypeError(name, moduleParam->getTypeInfo(), ModuleParam<T>::TypeInfo());
    } else
      throwNotFoundError(name);
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  //                   Python API
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  template <typename PythonObject>
  void ModuleParamList::setParamPython(const std::string& name, const PythonObject& pyObj)
  {

    ModuleParamPtr p = getParameterPtr(name);
    p->setValueFromPythonObject(pyObj);
  }

  template <typename PythonObject>
  void ModuleParamList::getParamValuesPython(const std::string& name,
                                             PythonObject& pyOutput,
                                             bool defaultValues) const
  {
    try {
      ModuleParamPtr p = getParameterPtr(name);
      p->setValueToPythonObject(pyOutput, defaultValues);
    } catch (std::runtime_error& exc) {
      B2ERROR(exc.what());
    }
  }

} // end of Belle2 namespace
