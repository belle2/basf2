/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/trackRelation/BasicTrackRelationVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

BasicTrackRelationVarSet::BasicTrackRelationVarSet()
  : Super()
{
}

bool BasicTrackRelationVarSet::extract(const Relation<const CDCTrack>* ptrTrackRelation)
{
  bool extracted = extractNested(ptrTrackRelation);
  if (not extracted or not ptrTrackRelation) return false;

  const Relation<const CDCTrack>& trackPair = *ptrTrackRelation;

  const CDCTrack* ptrFromTrack = trackPair.getFrom();
  const CDCTrack* ptrToTrack = trackPair.getTo();

  const CDCTrack& fromTrack = *ptrFromTrack;
  const CDCTrack& toTrack = *ptrToTrack;

  ISuperLayer fromISuperLayer = fromTrack.back().getISuperLayer();
  ISuperLayer toISuperLayer = toTrack.front().getISuperLayer();
  std::pair<int, int> superLayerIdPair = std::minmax(fromISuperLayer, toISuperLayer);
  var<named("sl_id_pair")>() = superLayerIdPair.second * 10 + superLayerIdPair.first;
  var<named("delta_sl_id")>() = toISuperLayer - fromISuperLayer;
  var<named("from_size")>() = fromTrack.size();
  var<named("to_size")>() = toTrack.size();
  return true;
}
