/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segment/CDCRecoSegment2DVarSet.h>
#include <assert.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCRecoSegment2DVarSet::CDCRecoSegment2DVarSet() :
  VarSet<CDCRecoSegment2DVarNames>()
{
}

bool CDCRecoSegment2DVarSet::extract(const CDCRecoSegment2D* ptrSegment2D)
{
  bool extracted = extractNested(ptrSegment2D);
  if (not extracted or not ptrSegment2D) return false;

  var<named("size")>() = ptrSegment2D->size();
  var<named("superlayer_id")>() = ptrSegment2D->getISuperLayer();

  return true;
}
