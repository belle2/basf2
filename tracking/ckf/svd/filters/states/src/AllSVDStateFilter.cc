/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/states/AllSVDStateFilter.h>
#include <tracking/ckf/svd/entities/CKFToSVDState.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;


Weight AllSVDStateFilter::operator()(const BaseSVDStateFilter::Object& pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const CKFToSVDState*>>& previousStates = pair.first;
  CKFToSVDState* currentState = pair.second;

  if (previousStates.size() < 3) {
    // the path is to short (seed + 2 hits) to have too much overlap
    return 1.0;
  }

  const CKFToSVDState* previousToPreviousState = previousStates[previousStates.size() - 2];

  if (previousToPreviousState->getGeometricalLayer() == currentState->getGeometricalLayer()) {
    return NAN;
  }

  if (std::count(previousStates.begin(), previousStates.end(), currentState)) {
    B2FATAL("Have found a cycle!");
  }

  return 1.0;
}