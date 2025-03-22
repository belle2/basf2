/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>
#include <tracking/trackingUtilities/numerics/WithWeight.h>
#include <tracking/trackingUtilities/numerics/Weight.h>

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
      TrackingUtilities::Findlet<const TrackingUtilities::WithWeight<const AHit*>, TrackingUtilities::WithWeight<AHit*>> {
    private:
      /// Parent class
      using Super = TrackingUtilities::Findlet<const TrackingUtilities::WithWeight<const AHit*>, TrackingUtilities::WithWeight<AHit*>>;

    public:
      /// The object this filter refers to
      using Object = std::pair<const std::vector<TrackingUtilities::WithWeight<const AHit*>>, AHit*>;

      /// Apply the () operator to all pairs of hit and current path.
      void apply(const std::vector<TrackingUtilities::WithWeight<const AHit*>>& currentPath,
                 std::vector<TrackingUtilities::WithWeight<AHit*>>& childHits) override;

      /// The filter operator for this class
      virtual TrackingUtilities::Weight operator()(const Object& object);
    };

  }
}
