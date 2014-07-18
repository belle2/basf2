/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/MCAxialStereoSegmentPairFilter.h"

#include <framework/logging/Logger.h>

#include <tracking/cdcLocalTracking/mclookup/CDCMCSegmentLookUp.h>

#include <tracking/cdcLocalTracking/fitting/CDCRiemannFitter.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

MCAxialStereoSegmentPairFilter::MCAxialStereoSegmentPairFilter()
{


}

MCAxialStereoSegmentPairFilter::~MCAxialStereoSegmentPairFilter()
{



}



void MCAxialStereoSegmentPairFilter::clear()
{
}



void MCAxialStereoSegmentPairFilter::initialize()
{
}



void MCAxialStereoSegmentPairFilter::terminate()
{
}



CellWeight MCAxialStereoSegmentPairFilter::isGoodAxialStereoSegmentPair(const CDCAxialStereoSegmentPair& axialStereoSegmentPair, bool allowBackward) const
{
  const CDCAxialRecoSegment2D* ptrStartSegment = axialStereoSegmentPair.getStartSegment();
  const CDCAxialRecoSegment2D* ptrEndSegment = axialStereoSegmentPair.getEndSegment();

  if (ptrStartSegment == nullptr) {
    B2ERROR("MCAxialStereoSegmentPairFilter::isGoodAxialStereoSegmentPair invoked with nullptr as start segment");
    return NOT_A_CELL;
  }

  if (ptrEndSegment == nullptr) {
    B2ERROR("MCAxialStereoSegmentPairFilter::isGoodAxialStereoSegmentPair invoked with nullptr as end segment");
    return NOT_A_CELL;
  }

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();

  // Check if the segments are aligned correctly along the Monte Carlo track
  ForwardBackwardInfo pairFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrStartSegment, ptrEndSegment);
  if (pairFBInfo == INVALID_INFO) return NOT_A_CELL;

  if (pairFBInfo == FORWARD or (allowBackward and pairFBInfo == BACKWARD)) {
    // Final check for the distance between the segment
    Index startNPassedSuperLayers = mcSegmentLookUp.getLastNPassedSuperLayers(ptrStartSegment);
    if (startNPassedSuperLayers == INVALID_INDEX) return NOT_A_CELL;

    Index endNPassedSuperLayers = mcSegmentLookUp.getFirstNPassedSuperLayers(ptrEndSegment);
    if (endNPassedSuperLayers == INVALID_INDEX) return NOT_A_CELL;

    //if (abs(startNPassedSuperLayers - endNPassedSuperLayers) > 1) return NOT_A_CELL;

    //do fits?
    return startSegment.size() + endSegment.size();
  }

  return NOT_A_CELL;

}
