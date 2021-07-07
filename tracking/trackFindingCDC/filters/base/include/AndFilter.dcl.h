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
    /// Filter adapter type that joins two filter results in an and like fashion
    template<class AFilter>
    class AndFilter : public AFilter {

    private:
      /// Type of the base class
      using Super = AFilter;

    public:
      /// Constructor from two filters to be joined
      AndFilter(std::unique_ptr<AFilter> lhsFilter, std::unique_ptr<AFilter> rhsFilter);

      /// Default destructor
      ~AndFilter();

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Return result of right hand side filter if left hand side filter acknowledges.
      Weight operator()(const typename AFilter::Object& obj) final;

    private:
      /// Left hand side filter
      std::unique_ptr<AFilter> m_lhsFilter;

      /// Right hand side filter
      std::unique_ptr<AFilter> m_rhsFilter;
    };
  }
}
