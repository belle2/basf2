/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcToSpacePoint/result/SimpleCDCVXDTrackCombinationFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight SimpleCDCVXDTrackCombinationFilter::operator()(const BaseCDCVXDTrackCombinationFilter::Object& pair)
{
  const std::vector<const SpacePoint*> spacePoints = pair.getHits();

  return spacePoints.size();
}
