/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/numerics/WithWeight.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  /// Simple findlet for searching the best candidate for a given seed aplying the given filter.
  template<class AFilter>
  class OverlapResolver : public TrackFindingCDC::Findlet<typename AFilter::Object, typename AFilter::Object> {
  public:
    /// The object to filter
    using Object = typename AFilter::Object;

    /// The parent class
    using Super = TrackFindingCDC::Findlet<typename AFilter::Object, typename AFilter::Object>;

    /// Construct this findlet and add the subfindlet as listener
    OverlapResolver();

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /// For each seed, search for the best candidate and return it.
    void apply(std::vector<Object>& results, std::vector<Object>& filteredResult) override;

  private:
    /// Subfindlet for filtering
    AFilter m_filter;

    // Temporary vectors
    /// temporary results vector with weights, out of which the overlaps will be build.
    std::vector<TrackFindingCDC::WithWeight<Object*>> m_resultsWithWeight;

    // Parameters
    /// Parameter: Enable overlap
    bool m_param_enableOverlapResolving = true;
    /// Parameter: In seed mode, use only the best seeds
    unsigned long m_param_useBestNInSeed = 3;
  };
}