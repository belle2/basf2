/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/FilterVarSet.dcl.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/core/ModuleParam.h>

#include <RtypesCore.h>

#include <algorithm>
#include <string>
#include <memory>
#include <cmath>
#include <cstddef>

namespace Belle2 {
  namespace TrackFindingCDC {

    constexpr
    static char const* const filterVarNames[] = {
      "accept",
      "weight",
      "positive",
    };

    template<class AFilter>
    const std::size_t FilterVarNames<AFilter>::nVars = size(filterVarNames);

    template<class AFilter>
    constexpr char const* FilterVarNames<AFilter>::getName(int iName)
    {
      return filterVarNames[iName];
    }

    template <class AFilter>
    FilterVarSet<AFilter>::FilterVarSet(const std::string& filterName)
      : Super()
      , m_filterName(filterName)
      , m_filterNamePrefix(filterName + '_')
      , m_ptrFilter(std::make_unique<Filter>())
    {
    }

    template <class AFilter>
    FilterVarSet<AFilter>::FilterVarSet(const std::string& filterName,
                                        std::unique_ptr<Filter> ptrFilter)
      : Super()
      , m_filterName(filterName)
      , m_filterNamePrefix(filterName + '_')
      , m_ptrFilter(std::move(ptrFilter))
    {
    }

    template <class AFilter>
    bool FilterVarSet<AFilter>::extract(const Object* obj)
    {
      bool extracted = Super::extract(obj);
      if (m_ptrFilter and obj) {
        Weight weight = (*m_ptrFilter)(*obj);
        this->template var<named("weight")>() = weight;
        this->template var<named("accept")>() = not std::isnan(weight) and not(weight < m_cut);
        this->template var<named("positive")>() = weight > 0 and not(weight < m_cut);

        // Forward the nested result.
        return extracted;
      } else {
        return false;
      }
    }

    template <class AFilter>
    void FilterVarSet<AFilter>::initialize()
    {
      ModuleParamList moduleParamList;
      const std::string prefix = "";
      m_ptrFilter->exposeParameters(&moduleParamList, prefix);

      // try to find the MVAFilter cut parameter and reset it such that we can set it
      std::vector<std::string> paramNames = moduleParamList.getParameterNames();
      if (std::count(paramNames.begin(), paramNames.end(), "cut")) {
        ModuleParam<double>& cutParam = moduleParamList.getParameter<double>("cut");
        m_cut = cutParam.getValue();
        cutParam.setDefaultValue(NAN);
      }

      this->addProcessingSignalListener(m_ptrFilter.get());
      Super::initialize();
    }

    template <class AFilter>
    std::vector<Named<Float_t*>> FilterVarSet<AFilter>::getNamedVariables(const std::string& prefix)
    {
      return Super::getNamedVariables(prefix + m_filterNamePrefix);
    }

    template <class AFilter>
    MayBePtr<Float_t> FilterVarSet<AFilter>::find(const std::string& varName)
    {
      if (varName.find(m_filterNamePrefix) == 0) {
        std::string varNameWithoutPrefix = varName.substr(m_filterNamePrefix.size());
        MayBePtr<Float_t> found = Super::find(varNameWithoutPrefix);
        if (found) return found;
      }
      return nullptr;
    }
  }
}
