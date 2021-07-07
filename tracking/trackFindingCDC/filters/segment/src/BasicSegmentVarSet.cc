/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segment/BasicSegmentVarSet.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool BasicSegmentVarSet::extract(const CDCSegment2D* ptrSegment2D)
{
  if (not ptrSegment2D) return false;

  var<named("size")>() = ptrSegment2D->size();
  var<named("superlayer_id")>() = ptrSegment2D->getISuperLayer();

  return true;
}
