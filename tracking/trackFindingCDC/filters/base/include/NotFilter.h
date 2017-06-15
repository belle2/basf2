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
    /// Filter adapter type that inverts the acceptance criterion for cross checks
    template<class AFilter>
    class NotFilter : public AFilter {
      /// Type of the base class
      using Super = AFilter;

    public:
      /// Constructor from filter to be inverted
      NotFilter(std::unique_ptr<AFilter> filter)
        : m_filter(std::move(filter))
      {
        this->addProcessingSignalListener(m_filter.get());
      }

      /// Accept inverse
      Weight operator()(const typename AFilter::Object& obj) final {
        Weight result = (*m_filter)(obj);
        if (std::isnan(result))
        {
          return 1;
        } else {
          return NAN;
        }
      }

    private:
      /// Filter to be inverted
      std::unique_ptr<AFilter> m_filter;
    };
  }
}
