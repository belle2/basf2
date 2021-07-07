/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/trackRelation/BasicTrackRelationVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool BasicTrackRelationVarSet::extract(const Relation<const CDCTrack>* ptrTrackRelation)
{
  if (not ptrTrackRelation) return false;

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
