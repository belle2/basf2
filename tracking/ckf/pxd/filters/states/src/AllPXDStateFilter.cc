/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/states/AllPXDStateFilter.h>
#include <tracking/ckf/pxd/entities/CKFToPXDState.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;


Weight AllPXDStateFilter::operator()(const BasePXDStateFilter::Object& pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const CKFToPXDState*>>& previousStates = pair.first;
  CKFToPXDState* currentState = pair.second;

  if (previousStates.size() < 3) {
    // the path is to short (seed + 2 hits) to have too much overlap
    return 1.0;
  }

  const CKFToPXDState* previousToPreviousState = previousStates[previousStates.size() - 2];

  if (previousToPreviousState->getGeometricalLayer() == currentState->getGeometricalLayer()) {
    return NAN;
  }

  if (std::count(previousStates.begin(), previousStates.end(), currentState)) {
    B2FATAL("Have found a cycle!");
  }

  return 1.0;
}