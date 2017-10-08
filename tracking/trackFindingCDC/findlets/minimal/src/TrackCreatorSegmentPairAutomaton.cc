/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSegmentPairAutomaton.h>

#include <tracking/trackFindingCDC/fitting/CDCAxialStereoFusion.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/ca/Path.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string TrackCreatorSegmentPairAutomaton::getDescription()
{
  return "Constructs tracks by extraction of segment pair paths in a cellular automaton.";
}

void TrackCreatorSegmentPairAutomaton::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_cellularPathFinder.exposeParameters(moduleParamList, prefix);
}

void TrackCreatorSegmentPairAutomaton::apply(
  const std::vector<CDCSegmentPair>& inputSegmentPairs,
  const std::vector<WeightedRelation<const CDCSegmentPair>>& inputSegmentPairRelations,
  std::vector<CDCTrack>& outputTracks)
{
  // Obtain the segment pairs as pointers
  std::vector<const CDCSegmentPair*> segmentPairPtrs =
    as_pointers<const CDCSegmentPair>(inputSegmentPairs);

  m_segmentPairPaths.clear();
  m_cellularPathFinder.apply(segmentPairPtrs, inputSegmentPairRelations, m_segmentPairPaths);

  // Reduce to plain tracks
  CDCAxialStereoFusion fusionFitter;
  for (const Path<const CDCSegmentPair>& segmentPairPath : m_segmentPairPaths) {
    CDCTrajectory3D startTrajectory3D = segmentPairPath.front()->getTrajectory3D();
    CDCTrajectory3D endTrajectory3D = segmentPairPath.back()->getTrajectory3D();

    // Work around for in stable fusion fits that reach over the apogee
    auto saverStereoApogeePositions = [&fusionFitter](const CDCSegmentPair * lhs,
    const CDCSegmentPair * rhs) {
      // Segment pairs are at a turning point
      if (lhs->getFromSegment()->getISuperLayer() != rhs->getToSegment()->getISuperLayer()) return false;

      // Only apply workaround if apogee is in a stereo superlayer
      if (lhs->getToSegment()->isAxial()) return false;

      // Reset critical situation to simpler fit
      fusionFitter.fusePreliminary(*lhs);
      fusionFitter.fusePreliminary(*rhs);

      // Return false to continue looking
      return false;
    };
    std::adjacent_find(segmentPairPath.begin(), segmentPairPath.end(), saverStereoApogeePositions);

    // Compute the average recohits with the preliminary fits
    outputTracks.push_back(CDCTrack::condense(segmentPairPath));
    CDCTrack& track = outputTracks.back();

    // Set the start and end trajectory from the original fits
    // Only necessary for the work around
    {
      Vector3D startPos = track.getStartTrajectory3D().getLocalOrigin();
      startTrajectory3D.setLocalOrigin(startPos);
      track.setStartTrajectory3D(startTrajectory3D);

      Vector3D endPos = track.getEndTrajectory3D().getLocalOrigin();
      endTrajectory3D.setLocalOrigin(endPos);
      track.setEndTrajectory3D(endTrajectory3D);
    }
    for (const CDCSegmentPair* segmentPair : segmentPairPath) {
      segmentPair->getFromSegment()->getAutomatonCell().setTakenFlag();
      segmentPair->getToSegment()->getAutomatonCell().setTakenFlag();
    }
  }
}
