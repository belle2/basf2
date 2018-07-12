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

#include <tracking/trackFindingCDC/mclookup/CDCMCSegment2DLookUp.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.icc.h>

#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::MCSymmetric<BaseSegmentRelationFilter>;

MCSegmentRelationFilter::MCSegmentRelationFilter(bool allowReverse)
  : Super(allowReverse)
{
}

void MCSegmentRelationFilter::exposeParameters(ModuleParamList* moduleParamList,
                                               const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);
  moduleParamList->addParameter(prefixed(prefix, "requireRLPure"),
                                m_param_requireRLPure,
                                "Switch to require the segment combination contain mostly correct rl information",
                                m_param_requireRLPure);

}

Weight MCSegmentRelationFilter::operator()(const CDCSegment2D& fromSegment,
                                           const CDCSegment2D& toSegment)
{
  const CDCMCSegment2DLookUp& mcSegmentLookUp = CDCMCSegment2DLookUp::getInstance();

  // Check if the segments are aligned correctly along the Monte Carlo track
  EForwardBackward pairFBInfo =
    m_param_requireRLPure
    ? mcSegmentLookUp.areAlignedInMCTrackWithRLCheck(&fromSegment, &toSegment)
    : mcSegmentLookUp.areAlignedInMCTrack(&fromSegment, &toSegment);

  if (pairFBInfo == EForwardBackward::c_Invalid) return NAN;

  if (pairFBInfo == EForwardBackward::c_Forward or
      (getAllowReverse() and pairFBInfo == EForwardBackward::c_Backward)) {

    // Final check for the distance between the segment
    Index fromNPassedSuperLayers = mcSegmentLookUp.getLastNPassedSuperLayers(&fromSegment);
    if (fromNPassedSuperLayers == c_InvalidIndex) return NAN;

    Index toNPassedSuperLayers = mcSegmentLookUp.getFirstNPassedSuperLayers(&toSegment);
    if (toNPassedSuperLayers == c_InvalidIndex) return NAN;

    // Allow relations only from the same super layer
    if (fromNPassedSuperLayers != toNPassedSuperLayers) return NAN;

    return fromSegment.size() + toSegment.size();
  }

  return NAN;
}
