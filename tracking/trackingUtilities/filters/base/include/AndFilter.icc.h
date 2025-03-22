/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/AndFilter.dcl.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <class AFilter>
    AndFilter<AFilter>::AndFilter(std::unique_ptr<AFilter> lhsFilter,
                                  std::unique_ptr<AFilter> rhsFilter)
      : m_lhsFilter(std::move(lhsFilter))
      , m_rhsFilter(std::move(rhsFilter))
    {
      this->addProcessingSignalListener(m_lhsFilter.get());
      this->addProcessingSignalListener(m_rhsFilter.get());
    }

    template <class AFilter>
    AndFilter<AFilter>::~AndFilter() = default;

    template <class AFilter>
    void AndFilter<AFilter>::exposeParameters(ModuleParamList* moduleParamList,
                                              const std::string& prefix)
    {
      if (m_lhsFilter) m_lhsFilter->exposeParameters(moduleParamList, prefix);
      if (m_rhsFilter) m_rhsFilter->exposeParameters(moduleParamList, prefix);
    }

    template<class AFilter>
    Weight AndFilter<AFilter>::operator()(const typename AFilter::Object& obj)
    {
      Weight lhsResult = (*m_lhsFilter)(obj);

      if (std::isnan(lhsResult)) {
        return NAN;
      } else {
        Weight rhsResult = (*m_rhsFilter)(obj);
        return rhsResult;
      }
    }
  }
}
