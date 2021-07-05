/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/findlets/OnHitApplier.dcl.h>
#include <tracking/trackFindingCDC/numerics/WeightComperator.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

namespace Belle2 {
  namespace vxdHoughTracking {

    template <class AHit>
    void OnHitApplier<AHit>::apply(const std::vector<TrackFindingCDC::WithWeight<const AHit*>>& currentPath,
                                   std::vector<TrackFindingCDC::WithWeight<AHit*>>& childHits)
    {
      if (childHits.empty()) {
        return;
      }

      for (TrackFindingCDC::WithWeight<AHit*>& hitWithWeight : childHits) {
        AHit& hit = *hitWithWeight;
        const TrackFindingCDC::Weight& weight = this->operator()({currentPath, &hit});
        hitWithWeight.setWeight(weight);
      }

      TrackFindingCDC::erase_remove_if(childHits, TrackFindingCDC::HasNaNWeight());
    };

    template <class AHit>
    TrackFindingCDC::Weight OnHitApplier<AHit>::operator()(const Object& object __attribute__((unused)))
    {
      return NAN;
    };

  }
}
