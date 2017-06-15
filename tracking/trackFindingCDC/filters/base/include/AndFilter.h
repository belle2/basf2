/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter adapter type that joins two filter results in an and like fashion
    template<class AFilter>
    class AndFilter : public AFilter {
      /// Type of the base class
      using Super = AFilter;

    public:
      /// Constructor from two filters to be joined
      AndFilter(std::unique_ptr<AFilter> lhsFilter, std::unique_ptr<AFilter> rhsFilter)
        : m_lhsFilter(std::move(lhsFilter))
        , m_rhsFilter(std::move(rhsFilter))
      {
        this->addProcessingSignalListener(m_lhsFilter.get());
        this->addProcessingSignalListener(m_rhsFilter.get());
      }

      /// Return result of right hand side filter if left hand side filter acknowledges.
      Weight operator()(const typename AFilter::Object& obj) final {
        Weight lhsResult = (*m_lhsFilter)(obj);
        Weight rhsResult = (*m_rhsFilter)(obj);

        if (std::isnan(lhsResult))
        {
          return NAN;
        } else {
          return rhsResult;
        }
      }

    private:
      /// Left hand side filter
      std::unique_ptr<AFilter> m_lhsFilter;

      /// Right hand side filter
      std::unique_ptr<AFilter> m_rhsFilter;
    };
  }
}
