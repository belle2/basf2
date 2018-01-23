/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/axialSegmentPair/BasicAxialSegmentPairVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/Functional.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  std::array<int, 8> getNHitsByILayer(const CDCSegment2D& segment)
  {
    std::array<int, 8> result{};
    for (const CDCRecoHit2D& recoHit2D : segment) {
      ++result[recoHit2D.getWireHit().getILayer()];
    }
    return result;
  }
}

bool BasicAxialSegmentPairVarSet::extract(const CDCAxialSegmentPair* ptrAxialSegmentPair)
{
  if (not ptrAxialSegmentPair) return false;

  const CDCAxialSegmentPair& axialSegmentPair = *ptrAxialSegmentPair;

  const CDCSegment2D* ptrFromSegment = axialSegmentPair.getStartSegment();
  const CDCSegment2D* ptrToSegment = axialSegmentPair.getEndSegment();

  const CDCSegment2D& fromSegment = *ptrFromSegment;
  const CDCSegment2D& toSegment = *ptrToSegment;

  var<named("stereo_kind")>() = static_cast<float>(fromSegment.getStereoKind());
  var<named("sl_id")>() = fromSegment.getISuperLayer();

  var<named("from_size")>() = fromSegment.size();
  var<named("to_size")>() = toSegment.size();

  std::array<int, 8> fromNHitsByILayer = getNHitsByILayer(fromSegment);
  std::array<int, 8> toNHitsByILayer = getNHitsByILayer(toSegment);

  var<named("from_n_layers")>() = std::count_if(fromNHitsByILayer.begin(), fromNHitsByILayer.end(), Id() > 0);;
  var<named("to_n_layers")>() = std::count_if(toNHitsByILayer.begin(), toNHitsByILayer.end(), Id() > 0);

  return true;
}
