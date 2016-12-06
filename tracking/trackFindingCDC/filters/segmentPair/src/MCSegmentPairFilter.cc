/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/MCSegmentPairFilter.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCSegment2DLookUp.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>

using namespace Belle2;
using namespace TrackFindingCDC;

MCSegmentPairFilter::MCSegmentPairFilter(bool allowReverse)
  : Super(allowReverse)
{
}

Weight MCSegmentPairFilter::operator()(const CDCSegmentPair& segmentPair)
{
  const CDCSegment2D* ptrFromSegment = segmentPair.getFromSegment();
  const CDCSegment2D* ptrToSegment = segmentPair.getToSegment();

  assert(ptrFromSegment);
  assert(ptrToSegment);

  const CDCSegment2D& fromSegment = *ptrFromSegment;
  const CDCSegment2D& toSegment = *ptrToSegment;

  if (fromSegment.size() < 4 or toSegment.size() < 4) return NAN;

  const CDCMCSegment2DLookUp& mcSegmentLookUp = CDCMCSegment2DLookUp::getInstance();

  // Check if the segments are aligned correctly along the Monte Carlo track
  EForwardBackward pairFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrFromSegment, ptrToSegment);
  if (pairFBInfo == EForwardBackward::c_Invalid) return NAN;

  if (pairFBInfo == EForwardBackward::c_Forward or
      (getAllowReverse() and pairFBInfo == EForwardBackward::c_Backward)) {
    // Final check for the distance between the segment
    Index fromNPassedSuperLayers = mcSegmentLookUp.getLastNPassedSuperLayers(ptrFromSegment);
    if (fromNPassedSuperLayers == c_InvalidIndex) return NAN;

    Index toNPassedSuperLayers = mcSegmentLookUp.getFirstNPassedSuperLayers(ptrToSegment);
    if (toNPassedSuperLayers == c_InvalidIndex) return NAN;

    if (abs(fromNPassedSuperLayers - toNPassedSuperLayers) > 1) return NAN;

    // Do fits
    if (not segmentPair.getTrajectory3D().isFitted()) {
      CDCTrajectory3D mcTrajectory3D = mcSegmentLookUp.getTrajectory3D(ptrFromSegment);
      if (pairFBInfo == EForwardBackward::c_Backward) {
        mcTrajectory3D.reverse();
      }
      segmentPair.setTrajectory3D(mcTrajectory3D);
    }
    return pairFBInfo * static_cast<Weight>(fromSegment.size() + toSegment.size());
  }
  return NAN;
}
