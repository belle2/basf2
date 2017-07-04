/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcToSpacePoint/result/Chi2CDCVXDTrackCombinationFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight Chi2CDCVXDTrackCombinationFilter::operator()(const BaseCDCVXDTrackCombinationFilter::Object& pair)
{
  return -pair.getWeight();
}
