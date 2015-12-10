/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/trackRelation/MCTrackRelationFilter.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


NeighborWeight MCTrackRelationFilter::operator()(const CDCTrack& fromTrack,
                                                 const CDCTrack& toTrack)
{
  const CDCMCTrackLookUp& mcTrackLookUp = CDCMCTrackLookUp::getInstance();

  // Check if the tracks are aligned correctly along the Monte Carlo track
  EForwardBackward pairFBInfo = mcTrackLookUp.areAlignedInMCTrack(&fromTrack, &toTrack);
  if (pairFBInfo == EForwardBackward::c_Invalid) return NOT_A_CELL;

  if (pairFBInfo == EForwardBackward::c_Forward or (getAllowReverse() and pairFBInfo == EForwardBackward::c_Backward)) {
    // Final check for the distance between the track
    Index fromNPassedSuperLayers = mcTrackLookUp.getLastNPassedSuperLayers(&fromTrack);
    if (fromNPassedSuperLayers == c_InvalidIndex) return NOT_A_CELL;

    Index toNPassedSuperLayers = mcTrackLookUp.getFirstNPassedSuperLayers(&toTrack);
    if (toNPassedSuperLayers == c_InvalidIndex) return NOT_A_CELL;

    if (fromNPassedSuperLayers == toNPassedSuperLayers) return NOT_A_CELL;

    return fromTrack.size() + toTrack.size();
  }

  return NOT_A_CELL;
}
