/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/numerics/WithWeight.h>

namespace Belle2 {
  class ModuleParamList;

  template<class AFilter>
  class OverlapResolver : public TrackFindingCDC::Findlet<typename AFilter::Object> {
  public:
    /// The object to filter
    using Object = typename AFilter::Object;

    /// The parent class
    using Super = TrackFindingCDC::Findlet<typename AFilter::Object>;

    /// Construct this findlet and add the subfindlet as listener
    OverlapResolver();

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /**
     */
    void apply(std::vector<Object>& results) override;

  private:
    /// Subfindlet for filtering
    AFilter m_filter;

    // Temporary vectors
    /// temporary results vector, that will be swapped with the real results vector.
    std::vector<Object> m_temporaryResults;
    /// temporary results vector with weights, out of which the overlaps will be build.
    std::vector<TrackFindingCDC::WithWeight<Object*>> m_resultsWithWeight;

    // Parameters
    /// Parameter: Enable overlap
    bool m_param_enableOverlapResolving = true;
    /// Parameter: In seed mode, use only the best seeds
    unsigned long m_param_useBestNInSeed = 3;
  };
}