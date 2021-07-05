/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentRelation/BasicSegmentRelationVarSet.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/utilities/Functional.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  std::array<int, 8> getNHitsByILayer(const CDCSegment2D& segment)
  {
    std::array<int, 8> result{};
    for (const CDCRecoHit2D& recoHit2D : segment) {
      ++result[recoHit2D.getWire().getILayer()];
    }
    return result;
  }
}

bool BasicSegmentRelationVarSet::extract(const Relation<const CDCSegment2D>* ptrSegmentRelation)
{
  if (not ptrSegmentRelation) return false;

  const Relation<const CDCSegment2D>& segmentPair = *ptrSegmentRelation;

  const CDCSegment2D* ptrFromSegment = segmentPair.getFrom();
  const CDCSegment2D* ptrToSegment = segmentPair.getTo();

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
