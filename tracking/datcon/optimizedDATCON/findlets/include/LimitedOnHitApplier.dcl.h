/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/datcon/optimizedDATCON/findlets/OnHitApplier.dcl.h>
#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  /**
   * Specialisation of the OnHitApplier, which
   * (a) uses a filter for the () operator, which is configurable
   * (b) does only allow for the best N candidates in the child hits. All other hits will be deleted.
   *
   * If m_param_useNHits is 0, all hits are used.
   */
  template <class AHit, class AFilter>
  class LimitedOnHitApplier : public OnHitApplier<AHit> {
  private:
    /// Parent class
    using Super = OnHitApplier<AHit>;

    /// The object to filer
    using Object = typename Super::Object;

  public:
    /// Constructor adding the findlet as a listener.
    LimitedOnHitApplier();

    /// Apply the filter to each pair of hits and current path and let only pass the best N hits.
    void apply(const std::vector<TrackFindingCDC::WithWeight<const AHit*>>& currentPath,
               std::vector<TrackFindingCDC::WithWeight<AHit*>>& childHits) override;

    /// Copy the filter operator to this method
    TrackFindingCDC::Weight operator()(const Object& object) override;

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override ;

  private:
    /// Parameter how many objects should pass maximal
    int m_param_useNHits = 0;

    /// Filter to decide on the hits
    AFilter m_filter;
  };
}
