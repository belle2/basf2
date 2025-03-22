/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
