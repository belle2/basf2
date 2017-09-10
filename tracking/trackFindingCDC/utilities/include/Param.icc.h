/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/Param.h>

#include <framework/core/ModuleParamList.h>

#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <class T>
    Param<T>::Param(const std::string& name, T* variable, const std::string& description, T value)
      : ParamBase(name, description, false)
      , m_variable(variable)
      , m_defaultValue(std::move(value))
    {
    }

    template <class T>
    Param<T>::Param(const std::string& name, T* variable, const std::string& description)
      : ParamBase(name, description, true)
      , m_variable(variable)
      , m_defaultValue()
    {
    }

    template <class T>
    Param<T>::~Param() = default;

    template <class T>
    T Param<T>::getValue()
    {
      return *m_variable;
    }

    template <class T>
    void
    Param<T>::setValue(T t)
    {
      *m_variable = std::move(t);
    }

    template <class T>
    void
    Param<T>::setDefaultValue(T t)
    {
      m_defaultValue = t;
      *m_variable = std::move(t);
    }

    template <class T>
    void
    Param<T>::transferTo(ModuleParamList* moduleParamList)
    {
      if (this->isForced()) {
        moduleParamList->addParameter(this->getName(), *m_variable, this->getDescription());
      } else {
        moduleParamList->addParameter(this->getName(),
                                      *m_variable,
                                      this->getDescription(),
                                      m_defaultValue);
      }
    }
  }
}
