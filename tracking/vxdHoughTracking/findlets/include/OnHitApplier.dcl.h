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
#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <vector>

namespace Belle2 {
  namespace vxdHoughTracking {

    /**
    * Helper findlet which applies its () operator to all pairs of path and hit with all hits in the given
    * child hit list. It deletes all hits in the list, where the operator () return NAN.
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

      /// Apply the () operator to all pairs of hit and current path.
      void apply(const std::vector<TrackFindingCDC::WithWeight<const AHit*>>& currentPath,
                 std::vector<TrackFindingCDC::WithWeight<AHit*>>& childHits) override;

      /// The filter operator for this class
      virtual TrackFindingCDC::Weight operator()(const Object& object);
    };

  }
}
