/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/collectors/stereo_segments/StereoSegmentTrackMatcherFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

// Not really sure why this is needed - but otherwise clang emits a linking error for this missing symbol.
template Weight Chooseable<BaseStereoSegmentFilter>::operator()(const Object&);


std::vector<WithWeight<const CDCSegment2D*> >
StereoSegmentTrackMatcherFilter::match(const CDCTrack& track,
                                       const std::vector<CDCSegment2D>& segments)
{
  if (m_filter.needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }

  // List of matches
  std::vector<WithWeight<const CDCSegment2D*>> matches;

  // Reconstruct the hits to the track and check their filter result
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();

  for (const CDCSegment2D& segment2D : segments) {
    if (segment2D.getStereoKind() != EStereoKind::c_Axial and
        not segment2D.getAutomatonCell().hasTakenFlag()
        and not segment2D.isFullyTaken(2)) {
      CDCSegment3D segment3D = CDCSegment3D::reconstruct(segment2D, trajectory2D);
      const Weight weight = m_filter({&segment3D, &track});
      if (not std::isnan(weight)) {
        matches.emplace_back(&segment2D, weight);
      }
    }
  }

  return matches;
}
