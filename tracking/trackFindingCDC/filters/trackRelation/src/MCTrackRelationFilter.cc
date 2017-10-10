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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::MCSymmetric<BaseTrackRelationFilter>;

MCTrackRelationFilter::MCTrackRelationFilter(bool allowReverse)
  : Super(allowReverse)
{
}

Weight MCTrackRelationFilter::operator()(const CDCTrack& fromTrack,
                                         const CDCTrack& toTrack)
{
  const CDCMCTrackLookUp& mcTrackLookUp = CDCMCTrackLookUp::getInstance();

  // Check if the tracks are aligned correctly along the Monte Carlo track
  EForwardBackward pairFBInfo = mcTrackLookUp.areAlignedInMCTrack(&fromTrack, &toTrack);
  if (pairFBInfo == EForwardBackward::c_Invalid) return NAN;

  if (pairFBInfo == EForwardBackward::c_Forward or
      (getAllowReverse() and pairFBInfo == EForwardBackward::c_Backward)) {
    // Final check for the distance between the track
    Index fromNPassedSuperLayers = mcTrackLookUp.getLastNPassedSuperLayers(&fromTrack);
    if (fromNPassedSuperLayers == c_InvalidIndex) return NAN;

    Index toNPassedSuperLayers = mcTrackLookUp.getFirstNPassedSuperLayers(&toTrack);
    if (toNPassedSuperLayers == c_InvalidIndex) return NAN;

    if (fromNPassedSuperLayers == toNPassedSuperLayers) return NAN;

    // Limit loops to be merged to one
    Index fromNLoops = mcTrackLookUp.getLastNLoops(&fromTrack);
    Index toNLoops = mcTrackLookUp.getFirstNLoops(&toTrack);
    if (fromNLoops != toNLoops) return NAN;

    return fromTrack.size() + toTrack.size();
  }

  return NAN;
}
