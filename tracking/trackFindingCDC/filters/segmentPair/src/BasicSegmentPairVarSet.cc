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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>

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

  CDCSegmentPair segmentPair = *ptrSegmentPair;

  var<named("axial_first")>() = segmentPair.getFromSegment()->isAxial();

  var<named("axial_size")>() = segmentPair.getAxialSegment()->size();
  var<named("stereo_size")>() = segmentPair.getStereoSegment()->size();

  ISuperLayer fromISuperLayer = segmentPair.getFromISuperLayer();
  ISuperLayer toISuperLayer = segmentPair.getToISuperLayer();

  std::pair<int, int> superLayerIdPair = std::minmax(fromISuperLayer, toISuperLayer);
  var<named("sl_id_pair")>() = superLayerIdPair.second * 10 + superLayerIdPair.first;

  return true;
}
