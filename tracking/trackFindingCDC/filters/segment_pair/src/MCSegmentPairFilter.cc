/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/segment_pair/MCSegmentPairFilter.h>

#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


CellWeight MCSegmentPairFilter::operator()(const CDCSegmentPair& segmentPair)
{
  const CDCAxialRecoSegment2D* ptrStartSegment = segmentPair.getStartSegment();
  const CDCAxialRecoSegment2D* ptrEndSegment = segmentPair.getEndSegment();

  assert(ptrStartSegment);
  assert(ptrEndSegment);

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();

  // Check if the segments are aligned correctly along the Monte Carlo track
  EForwardBackward pairFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrStartSegment, ptrEndSegment);
  if (pairFBInfo == EForwardBackward::c_Invalid) return NOT_A_CELL;

  if (pairFBInfo == EForwardBackward::c_Forward or (getAllowReverse() and pairFBInfo == EForwardBackward::c_Backward)) {
    // Final check for the distance between the segment
    Index startNPassedSuperLayers = mcSegmentLookUp.getLastNPassedSuperLayers(ptrStartSegment);
    if (startNPassedSuperLayers == INVALID_INDEX) return NOT_A_CELL;

    Index endNPassedSuperLayers = mcSegmentLookUp.getFirstNPassedSuperLayers(ptrEndSegment);
    if (endNPassedSuperLayers == INVALID_INDEX) return NOT_A_CELL;

    if (abs(startNPassedSuperLayers - endNPassedSuperLayers) > 1) return NOT_A_CELL;

    // Do fits
    CDCTrajectory3D mcTrajectory3D = mcSegmentLookUp.getTrajectory3D(ptrStartSegment);
    segmentPair.setTrajectory3D(mcTrajectory3D);

    return startSegment.size() + endSegment.size();
  }

  return NOT_A_CELL;

}
