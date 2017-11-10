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

#include <tracking/trackFindingCDC/mclookup/CDCMCSegment2DLookUp.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.icc.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::MCSymmetric<BaseAxialSegmentPairFilter>;

MCAxialSegmentPairFilter::MCAxialSegmentPairFilter(bool allowReverse)
  : Super(allowReverse)
{
}

void MCAxialSegmentPairFilter::exposeParameters(ModuleParamList* moduleParamList,
                                                const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);
  moduleParamList->addParameter(prefixed(prefix, "requireRLPure"),
                                m_param_requireRLPure,
                                "Switch to require the segment combination contain mostly correct rl information",
                                m_param_requireRLPure);
}

Weight MCAxialSegmentPairFilter::operator()(const CDCAxialSegmentPair& axialSegmentPair)
{
  const CDCAxialSegment2D* ptrFromSegment = axialSegmentPair.getStartSegment();
  const CDCAxialSegment2D* ptrToSegment = axialSegmentPair.getEndSegment();

  assert(ptrFromSegment);
  assert(ptrToSegment);

  const CDCAxialSegment2D& fromSegment = *ptrFromSegment;
  const CDCAxialSegment2D& toSegment = *ptrToSegment;

  const CDCMCSegment2DLookUp& mcSegmentLookUp = CDCMCSegment2DLookUp::getInstance();

  // Check if the segments are aligned correctly along the Monte Carlo track
  EForwardBackward pairFBInfo =
    m_param_requireRLPure
    ? mcSegmentLookUp.areAlignedInMCTrackWithRLCheck(ptrFromSegment, ptrToSegment)
    : mcSegmentLookUp.areAlignedInMCTrack(ptrFromSegment, ptrToSegment);

  if (pairFBInfo == EForwardBackward::c_Invalid) return NAN;

  if (pairFBInfo == EForwardBackward::c_Forward or (getAllowReverse() and pairFBInfo == EForwardBackward::c_Backward)) {
    // Final check for the distance between the segment
    Index fromNPassedSuperLayers = mcSegmentLookUp.getLastNPassedSuperLayers(ptrFromSegment);
    if (fromNPassedSuperLayers == c_InvalidIndex) return NAN;

    Index toNPassedSuperLayers = mcSegmentLookUp.getFirstNPassedSuperLayers(ptrToSegment);
    if (toNPassedSuperLayers == c_InvalidIndex) return NAN;

    // Do not join in the same superlayer - this should be the responsibility of the segment relation filter
    if (abs(fromNPassedSuperLayers - toNPassedSuperLayers) == 0) return NAN;

    // Do not join further away than two superlayers
    if (abs(fromNPassedSuperLayers - toNPassedSuperLayers) > 2) return NAN;

    if (not axialSegmentPair.getTrajectory2D().isFitted()) {
      CDCTrajectory2D trajectory2D = CDCRiemannFitter::getFitter().fit(fromSegment, toSegment);
      axialSegmentPair.setTrajectory2D(trajectory2D);
    }

    return fromSegment.size() + toSegment.size();
  }
  return NAN;
}
