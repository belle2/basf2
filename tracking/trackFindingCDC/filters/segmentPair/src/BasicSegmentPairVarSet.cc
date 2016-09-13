/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/BasicSegmentPairVarSet.h>
#include <assert.h>


using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

BasicSegmentPairVarSet::BasicSegmentPairVarSet()
  : Super()
{
}

bool BasicSegmentPairVarSet::extract(const CDCSegmentPair* ptrSegmentPair)
{
  bool extracted = extractNested(ptrSegmentPair);
  if (not extracted or not ptrSegmentPair) return false;

  var<named("axial_first")>() = ptrSegmentPair->getFromSegment()->isAxial();
  return true;
}
