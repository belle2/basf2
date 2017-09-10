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

#include <tracking/trackFindingCDC/utilities/ParamList.h>

#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <typename T>
    void ParamList::addParameter(const std::string& name,
                                 T& variable,
                                 const std::string& description,
                                 T value)
    {
      m_paramsByName[name] =
        std::make_unique<Param<T>>(name, &variable, description, std::move(value));
    }

    template <typename T>
    void ParamList::addParameter(const std::string& name,
                                 T& variable,
                                 const std::string& description)
    {
      m_paramsByName[name] = std::make_unique<Param<T>>(name, &variable, description);
    }

    template <typename T>
    Param<T>& ParamList::getParameter(const std::string& name)
    {
      auto itParam = m_paramsByName.find(name);
      B2ASSERT("Parameter with name " << name << " not found", itParam != m_paramsByName.end());
      ParamBase* ptrParamBase = itParam->second.get();
      B2ASSERT("A nullptr slipped into the map", ptrParamBase != nullptr);
      Param<T>* ptrParam = dynamic_cast<Param<T>*>(ptrParamBase);
      B2ASSERT("Bad parameter type cast", ptrParam != nullptr);
      return *ptrParam;
    }
  }
}
