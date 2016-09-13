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

#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


Weight MCSegmentPairFilter::operator()(const CDCSegmentPair& segmentPair)
{
  const CDCAxialRecoSegment2D* ptrStartSegment = segmentPair.getFromSegment();
  const CDCAxialRecoSegment2D* ptrEndSegment = segmentPair.getToSegment();

  assert(ptrStartSegment);
  assert(ptrEndSegment);

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  if (startSegment.size() < 4 or endSegment.size() < 4) return NAN;

  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();

  // Check if the segments are aligned correctly along the Monte Carlo track
  EForwardBackward pairFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrStartSegment, ptrEndSegment);
  if (pairFBInfo == EForwardBackward::c_Invalid) return NAN;

  if (pairFBInfo == EForwardBackward::c_Forward or (getAllowReverse() and pairFBInfo == EForwardBackward::c_Backward)) {
    // Final check for the distance between the segment
    Index startNPassedSuperLayers = mcSegmentLookUp.getLastNPassedSuperLayers(ptrStartSegment);
    if (startNPassedSuperLayers == c_InvalidIndex) return NAN;

    Index endNPassedSuperLayers = mcSegmentLookUp.getFirstNPassedSuperLayers(ptrEndSegment);
    if (endNPassedSuperLayers == c_InvalidIndex) return NAN;

    if (abs(startNPassedSuperLayers - endNPassedSuperLayers) > 1) return NAN;

    // Do fits
    if (not segmentPair.getTrajectory3D().isFitted()) {
      CDCTrajectory3D mcTrajectory3D = mcSegmentLookUp.getTrajectory3D(ptrStartSegment);
      if (pairFBInfo == EForwardBackward::c_Backward) {
        mcTrajectory3D.reverse();
      }
      segmentPair.setTrajectory3D(mcTrajectory3D);
    }

    return pairFBInfo * static_cast<Weight>(startSegment.size() + endSegment.size());
  }

  return NAN;

}
