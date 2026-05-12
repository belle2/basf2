/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/findlets/OnHitApplier.dcl.h>
#include <tracking/trackingUtilities/numerics/WeightComperator.h>
#include <tracking/trackingUtilities/utilities/Algorithms.h>

namespace Belle2 {
  namespace vxdHoughTracking {

    template <class AHit>
    void OnHitApplier<AHit>::apply(const std::vector<TrackingUtilities::WithWeight<const AHit*>>& currentPath,
                                   std::vector<TrackingUtilities::WithWeight<AHit*>>& childHits)
    {
      if (childHits.empty()) {
        return;
      }

      for (TrackingUtilities::WithWeight<AHit*>& hitWithWeight : childHits) {
        AHit& hit = *hitWithWeight;
        const TrackingUtilities::Weight& weight = this->operator()({currentPath, &hit});
        hitWithWeight.setWeight(weight);
      }

      TrackingUtilities::erase_remove_if(childHits, TrackingUtilities::HasNaNWeight());
    };

    template <class AHit>
    TrackingUtilities::Weight OnHitApplier<AHit>::operator()(const Object& object __attribute__((unused)))
    {
      return NAN;
    };

  }
}
