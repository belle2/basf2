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

#include <tracking/trackFindingCDC/filters/base/NotFilter.dcl.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <memory>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <class AFilter>
    NotFilter<AFilter>::NotFilter(std::unique_ptr<AFilter> filter)
      : m_filter(std::move(filter))
    {
      this->addProcessingSignalListener(m_filter.get());
    }

    template <class AFilter>
    NotFilter<AFilter>::~NotFilter() = default;

    template <class AFilter>
    void NotFilter<AFilter>::exposeParameters(ModuleParamList* moduleParamList,
                                              const std::string& prefix)
    {
      if (m_filter) m_filter->exposeParameters(moduleParamList, prefix);
    }

    template <class AFilter>
    Weight NotFilter<AFilter>::operator()(const typename AFilter::Object& obj)
    {
      Weight result = (*m_filter)(obj);
      if (std::isnan(result)) {
        return 1;
      } else {
        return NAN;
      }
    }
  }
}
