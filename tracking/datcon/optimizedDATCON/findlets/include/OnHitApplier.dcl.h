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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <vector>

namespace Belle2 {

  /**
   * Helper findlet which applies its () operator to all pairs of path and state with all states in the given
   * child state list. It deletes all states in the list, where the operator () return NAN.
   * Should probably be overloaded in derived classes.
   */
  template <class AHit>
  class OnHitApplier : public
    TrackFindingCDC::Findlet<const TrackFindingCDC::WithWeight<const AHit*>, TrackFindingCDC::WithWeight<AHit*>> {
  private:
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const TrackFindingCDC::WithWeight<const AHit*>, TrackFindingCDC::WithWeight<AHit*>>;

  public:
    /// The object this filter refers to
    using Object = std::pair<const std::vector<TrackFindingCDC::WithWeight<const AHit*>>, AHit*>;

    /// Apply the () operator to all pairs of state and current path.
    void apply(const std::vector<TrackFindingCDC::WithWeight<const AHit*>>& currentPath,
               std::vector<TrackFindingCDC::WithWeight<AHit*>>& childHits) override;

    /// The filter operator for this class
    virtual TrackFindingCDC::Weight operator()(const Object& object);
  };
}
