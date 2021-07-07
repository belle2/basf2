/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      explicit NotFilter(std::unique_ptr<AFilter> filter);

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
