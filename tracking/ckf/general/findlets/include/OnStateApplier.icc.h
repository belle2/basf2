/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/findlets/OnStateApplier.dcl.h>
#include <tracking/trackFindingCDC/numerics/WeightComperator.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

namespace Belle2 {
  template <class AState>
  void OnStateApplier<AState>::apply(const std::vector<TrackFindingCDC::WithWeight<const AState*>>& currentPath,
                                     std::vector<TrackFindingCDC::WithWeight<AState*>>& childStates)
  {
    if (childStates.empty()) {
      return;
    }

    for (TrackFindingCDC::WithWeight<AState*>& stateWithWeight : childStates) {
      AState& state = *stateWithWeight;
      const TrackFindingCDC::Weight& weight = this->operator()({currentPath, &state});
      stateWithWeight.setWeight(weight);
    }

    TrackFindingCDC::erase_remove_if(childStates, TrackFindingCDC::HasNaNWeight());
  };

  template <class AState>
  TrackFindingCDC::Weight OnStateApplier<AState>::operator()(const Object& object __attribute__((unused)))
  {
    return NAN;
  };
}
