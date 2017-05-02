/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcTrackSpacePointCombination/SimpleCDCVXDTrackCombinationFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight SimpleCDCVXDTrackCombinationFilter::operator()(const BaseCDCVXDTrackCombinationFilter::Object& pair)
{
  const RecoTrack* recoTrack = pair.first;
  const std::vector<const SpacePoint*> spacePoints = pair.second;

  return spacePoints.size();
}
