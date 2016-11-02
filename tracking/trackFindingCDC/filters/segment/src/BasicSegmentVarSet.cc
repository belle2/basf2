/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segment/BasicSegmentVarSet.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool BasicSegmentVarSet::extract(const CDCRecoSegment2D* ptrSegment2D)
{
  bool extracted = extractNested(ptrSegment2D);
  if (not extracted or not ptrSegment2D) return false;

  var<named("size")>() = ptrSegment2D->size();
  var<named("superlayer_id")>() = ptrSegment2D->getISuperLayer();

  return true;
}
