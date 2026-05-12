/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/findlets/OnStateApplier.dcl.h>
#include <tracking/trackingUtilities/numerics/WeightComperator.h>
#include <tracking/trackingUtilities/utilities/Algorithms.h>

namespace Belle2 {
  template <class AState>
  void OnStateApplier<AState>::apply(const std::vector<TrackingUtilities::WithWeight<const AState*>>& currentPath,
                                     std::vector<TrackingUtilities::WithWeight<AState*>>& childStates)
  {
    if (childStates.empty()) {
      return;
    }

    for (TrackingUtilities::WithWeight<AState*>& stateWithWeight : childStates) {
      AState& state = *stateWithWeight;
      const TrackingUtilities::Weight& weight = this->operator()({currentPath, &state});
      stateWithWeight.setWeight(weight);
    }

    TrackingUtilities::erase_remove_if(childStates, TrackingUtilities::HasNaNWeight());
  };

  template <class AState>
  TrackingUtilities::Weight OnStateApplier<AState>::operator()(const Object& object __attribute__((unused)))
  {
    return NAN;
  };
}
