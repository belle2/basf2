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

    template<class AFilter>
    Weight AndFilter<AFilter>::operator()(const typename AFilter::Object& obj)
    {
      Weight lhsResult = (*m_lhsFilter)(obj);
      Weight rhsResult = (*m_rhsFilter)(obj);

      if (std::isnan(lhsResult)) {
        return NAN;
      } else {
        return rhsResult;
      }
    }
  }
}
