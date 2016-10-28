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

#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <utility>

using namespace Belle2;
using namespace TrackFindingCDC;

void StereoSegmentTrackMatcherQuadTree::exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix)
{
  QuadTreeBasedMatcher<SegmentZ0TanLambdaLegendre>::exposeParameters(moduleParameters, prefix);

  moduleParameters->addParameter(prefixed(prefix, "checkForB2BTracks"), m_param_checkForB2BTracks,
                                 "Set to false to skip the check for back-2-back tracks (good for cosmics)",
                                 m_param_checkForB2BTracks);
}

bool StereoSegmentTrackMatcherQuadTree::checkRecoSegment3D(CDCRecoSegment3D& recoSegment3D, const bool isCurler,
                                                           const double shiftValue, const ISuperLayer lastSuperLayer,
                                                           const double lastArcLength2D) const
{
  unsigned int numberOfHitsNotInCDCBounds = 0;
  unsigned int numberOfHitsOnWrongSide = 0;

  for (CDCRecoHit3D& recoHit : recoSegment3D) {
    if (not recoHit.isInCellZBounds(1.5)) {
      numberOfHitsNotInCDCBounds++;
    }

    if (recoHit.getArcLength2D() < 0) {
      if (not isCurler) {
        numberOfHitsOnWrongSide++;
      }
      recoHit.shiftArcLength2D(shiftValue);
    }
  }

  if (numberOfHitsNotInCDCBounds > 3) {
    return false;
  }

  if (recoSegment3D.size() <= 3 and numberOfHitsNotInCDCBounds > 1) {
    return false;
  }

  if (m_param_checkForB2BTracks and numberOfHitsOnWrongSide > 1) {
    return false;
  }

  if (recoSegment3D.back().getArcLength2D() > lastArcLength2D) {
    if (abs(lastSuperLayer - recoSegment3D.getISuperLayer()) > 1) {
      return false;
    }
  }

  return true;
}

std::vector<WithWeight<const CDCRecoSegment2D*>> StereoSegmentTrackMatcherQuadTree::match(const CDCTrack& track,
                                              const std::vector<CDCRecoSegment2D>& recoSegments)
{

  const CDCSZFitter& szFitter = CDCSZFitter::getFitter();

  typedef std::pair<std::pair<CDCRecoSegment3D, CDCTrajectorySZ>, const CDCRecoSegment2D*> CDCRecoSegment3DWithPointer;
  std::vector<CDCRecoSegment3DWithPointer> recoSegmentsWithPointer;
  recoSegmentsWithPointer.reserve(recoSegments.size());

  // Reconstruct the segments to the track
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const bool isCurler = trajectory2D.isCurler();
  const double shiftValue = trajectory2D.getLocalCircle()->arcLengthPeriod();
  const ISuperLayer lastSuperLayer = track.back().getISuperLayer();
  const double lastArcLength2D = track.back().getArcLength2D();

  for (const CDCRecoSegment2D& recoSegment2D : recoSegments) {
    if (not(recoSegment2D.getStereoKind() == EStereoKind::c_Axial or recoSegment2D.getAutomatonCell().hasTakenFlag()
            or recoSegment2D.isFullyTaken(2))) {
      CDCRecoSegment3D recoSegment3D = CDCRecoSegment3D::reconstruct(recoSegment2D, trajectory2D);

      if (checkRecoSegment3D(recoSegment3D, isCurler, shiftValue, lastSuperLayer, lastArcLength2D)) {
        const CDCTrajectorySZ& trajectorySZ = szFitter.fitUsingSimplifiedTheilSen(recoSegment3D);
        recoSegmentsWithPointer.emplace_back(std::make_pair(recoSegment3D, trajectorySZ), &recoSegment2D);
      }

      CDCRecoSegment3D recoSegment3DReversed = CDCRecoSegment3D::reconstruct(recoSegment2D.reversed(), trajectory2D);

      if (checkRecoSegment3D(recoSegment3DReversed, isCurler, shiftValue, lastSuperLayer, lastArcLength2D)) {
        const CDCTrajectorySZ& trajectorySZ = szFitter.fitUsingSimplifiedTheilSen(recoSegment3DReversed);
        recoSegmentsWithPointer.emplace_back(std::make_pair(recoSegment3DReversed, trajectorySZ), &recoSegment2D);
      }
    }
  }

  m_quadTreeInstance.seed(recoSegmentsWithPointer);

  if (m_param_writeDebugInformation) {
    writeDebugInformation();
  }

  const auto& foundStereoSegmentsWithNode = m_quadTreeInstance.findSingleBest(m_param_minimumNumberOfHits);
  m_quadTreeInstance.fell();


  if (foundStereoSegmentsWithNode.size() != 1) {
    return {};
  }

  const auto& foundStereoSegmentWithNode = foundStereoSegmentsWithNode[0];

  // List of matches
  std::vector<WithWeight<const CDCRecoSegment2D*>> matches;
  const auto& foundStereoSegments = foundStereoSegmentWithNode.second;

  matches.reserve(foundStereoSegments.size());

  for (const CDCRecoSegment3DWithPointer& segment3DWithPointer : foundStereoSegments) {
    matches.emplace_back(segment3DWithPointer.second, 1.0);
  }

  std::sort(matches.begin(), matches.end());
  matches.erase(std::unique(matches.begin(), matches.end()), matches.end());


  return matches;
}
