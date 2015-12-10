/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/segmentRelation/MCSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

NeighborWeight MCSegmentRelationFilter::operator()(const CDCRecoSegment2D& fromSegment,
                                                   const CDCRecoSegment2D& toSegment)
{
  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();

  // Check if the segments are aligned correctly along the Monte Carlo track
  EForwardBackward pairFBInfo = mcSegmentLookUp.areAlignedInMCTrack(&fromSegment, &toSegment);
  if (pairFBInfo == EForwardBackward::c_Invalid) return NOT_A_CELL;

  if (pairFBInfo == EForwardBackward::c_Forward or (getAllowReverse() and pairFBInfo == EForwardBackward::c_Backward)) {
    // Final check for the distance between the segment
    Index fromNPassedSuperLayers = mcSegmentLookUp.getLastNPassedSuperLayers(&fromSegment);
    if (fromNPassedSuperLayers == c_InvalidIndex) return NOT_A_CELL;

    Index toNPassedSuperLayers = mcSegmentLookUp.getFirstNPassedSuperLayers(&toSegment);
    if (toNPassedSuperLayers == c_InvalidIndex) return NOT_A_CELL;

    if (fromNPassedSuperLayers == toNPassedSuperLayers) return NOT_A_CELL;

    return fromSegment.size() + toSegment.size();
  }

  return NOT_A_CELL;
}
