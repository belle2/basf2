/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/collectors/stereo_segments/StereoSegmentTrackMatcherQuadTree.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <utility>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

void StereoSegmentTrackMatcherQuadTree::exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix)
{
  QuadTreeBasedMatcher<SegmentZ0TanLambdaLegendre>::exposeParameters(moduleParameters, prefix);

  moduleParameters->addParameter(prefix + "checkForB2BTracks", m_param_checkForB2BTracks,
                                 "Set to false to skip the check for back-2-back tracks (good for cosmics)",
                                 m_param_checkForB2BTracks);
}

std::vector<WithWeight<const CDCRecoSegment2D*>> StereoSegmentTrackMatcherQuadTree::match(const CDCTrack& track,
                                              const std::vector<CDCRecoSegment2D>& recoSegments)
{

  typedef std::pair<CDCRecoSegment3D, const CDCRecoSegment2D*> CDCRecoSegment3DWithPointer;
  std::vector<CDCRecoSegment3DWithPointer> recoSegmentsWithPointer;
  recoSegmentsWithPointer.reserve(recoSegments.size());

  // Reconstruct the segments to the track
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const bool isCurler = trajectory2D.isCurler();
  const double radius = trajectory2D.getLocalCircle().absRadius();
  const double shiftValue = 2 * TMath::Pi() * radius;

  for (const CDCRecoSegment2D& recoSegment2D : recoSegments) {
    if ((recoSegment2D.getStereoKind() == EStereoKind::c_StereoU or recoSegment2D.getStereoKind() == EStereoKind::c_StereoV)
        and not recoSegment2D.getAutomatonCell().hasTakenFlag() and not recoSegment2D.isFullyTaken(2)) {
      CDCRecoSegment3D recoSegment3D = CDCRecoSegment3D::reconstruct(recoSegment2D, trajectory2D);

      unsigned int numberOfHitsNotInCDCBounds = 0;
      unsigned int numberOfHitsOnWrongSide = 0;

      for (CDCRecoHit3D& recoHit : recoSegment3D) {
        if (not recoHit.isInCellZBounds(1.1)) {
          numberOfHitsNotInCDCBounds++;
        }

        if (recoHit.getArcLength2D() < 0) {
          if (not isCurler) {
            numberOfHitsOnWrongSide++;
          }
          recoHit.shiftArcLength2D(shiftValue);
        }
      }

      if (numberOfHitsNotInCDCBounds > 1) {
        continue;
      }

      if (m_param_checkForB2BTracks and numberOfHitsOnWrongSide > 1) {
        continue;
      }

      recoSegmentsWithPointer.emplace_back(recoSegment3D, &recoSegment2D);
    }
  }

  m_quadTreeInstance.seed(recoSegmentsWithPointer);
  const auto& foundStereoSegmentsWithNode = m_quadTreeInstance.findSingleBest(m_param_minimumNumberOfHits);
  m_quadTreeInstance.fell();

  if (foundStereoSegmentsWithNode.size() != 1) {
    return {};
  }

  const auto& foundStereoSegmentWithNode = foundStereoSegmentsWithNode[0];

  // List of matches
  std::vector<WithWeight<const CDCRecoSegment2D*>> matches;

  const auto& foundStereoSegments = foundStereoSegmentWithNode.second;

  for (const CDCRecoSegment3DWithPointer& segment3DWthPointer : foundStereoSegments) {
    matches.emplace_back(segment3DWthPointer.second, 1.0);
  }


  return matches;
}
