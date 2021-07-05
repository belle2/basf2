/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/BasicSegmentPairVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool BasicSegmentPairVarSet::extract(const CDCSegmentPair* ptrSegmentPair)
{
  if (not ptrSegmentPair) return false;

  CDCSegmentPair segmentPair = *ptrSegmentPair;

  const CDCSegment2D* fromSegment = segmentPair.getFromSegment();
  const CDCSegment2D* toSegment = segmentPair.getToSegment();

  const CDCSegment2D* axialSegment = segmentPair.getAxialSegment();
  const CDCSegment2D* stereoSegment = segmentPair.getStereoSegment();

  if (fromSegment->empty()) return false;
  if (toSegment->empty()) return false;

  var<named("axial_first")>() = fromSegment->back().isAxial();
  var<named("axial_size")>() = axialSegment->size();
  var<named("stereo_size")>() = stereoSegment->size();

  ISuperLayer fromISuperLayer = fromSegment->back().getISuperLayer();
  ISuperLayer toISuperLayer = toSegment->front().getISuperLayer();

  std::pair<int, int> superLayerIdPair = std::minmax(fromISuperLayer, toISuperLayer);
  var<named("sl_id_pair")>() = superLayerIdPair.second * 10 + superLayerIdPair.first;

  return true;
}
