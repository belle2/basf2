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

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <memory>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    /// Filter adapter type that inverts the acceptance criterion for cross checks
    template<class AFilter>
    class NotFilter : public AFilter {

    private:
      /// Type of the base class
      using Super = AFilter;

    public:
      /// Type of the object to be filtered
      using Object = typename AFilter::Object;

    public:
      /// Constructor from filter to be inverted
      NotFilter(std::unique_ptr<AFilter> filter);

      /// Default destructor.
      ~NotFilter();

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Accept inverse
      Weight operator()(const typename AFilter::Object& obj) final;

    private:
      /// Filter to be inverted
      std::unique_ptr<AFilter> m_filter;
    };
  }
}
