/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/axialSegmentPair/MCAxialSegmentPairFilter.h>

#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

MCAxialSegmentPairFilter::MCAxialSegmentPairFilter(bool allowReverse) :
  Super(allowReverse)
{
}


Weight MCAxialSegmentPairFilter::operator()(const CDCAxialSegmentPair& axialSegmentPair)
{
  const CDCAxialRecoSegment2D* ptrStartSegment = axialSegmentPair.getStartSegment();
  const CDCAxialRecoSegment2D* ptrEndSegment = axialSegmentPair.getEndSegment();

  assert(ptrStartSegment);
  assert(ptrEndSegment);

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();

  // Check if the segments are aligned correctly along the Monte Carlo track
  EForwardBackward pairFBInfo =
    mcSegmentLookUp.areAlignedInMCTrack(ptrStartSegment, ptrEndSegment);
  if (pairFBInfo == EForwardBackward::c_Invalid) return NAN;

  if (pairFBInfo == EForwardBackward::c_Forward or (getAllowReverse() and pairFBInfo == EForwardBackward::c_Backward)) {
    // Final check for the distance between the segment
    Index startNPassedSuperLayers = mcSegmentLookUp.getLastNPassedSuperLayers(ptrStartSegment);
    if (startNPassedSuperLayers == c_InvalidIndex) return NAN;

    Index endNPassedSuperLayers = mcSegmentLookUp.getFirstNPassedSuperLayers(ptrEndSegment);
    if (endNPassedSuperLayers == c_InvalidIndex) return NAN;

    if (abs(startNPassedSuperLayers - endNPassedSuperLayers) > 2) return NAN;

    //do fits?
    return startSegment.size() + endSegment.size();
  }

  return NAN;

}
