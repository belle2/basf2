/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/states/DistanceCDCStateFilter.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

using namespace Belle2;

TrackFindingCDC::Weight DistanceCDCStateFilter::operator()(const BaseCDCStateFilter::Object& pair)
{
  const CDCCKFState& state = *(pair.second);
  return 1 / std::abs(state.getHitDistance());
}