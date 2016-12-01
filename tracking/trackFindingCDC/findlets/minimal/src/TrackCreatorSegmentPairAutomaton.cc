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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>

#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>
#include <tracking/trackFindingCDC/ca/Path.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string TrackCreatorSegmentPairAutomaton::getDescription()
{
  return "Constructs tracks by extraction of segment pair paths in a cellular automaton.";
}

void TrackCreatorSegmentPairAutomaton::apply(
  const std::vector<CDCSegmentPair>& inputSegmentPairs,
  const std::vector<WeightedRelation<const CDCSegmentPair>>& inputSegmentPairRelations,
  std::vector<CDCTrack>& outputTracks)
{
  m_segmentPairPaths.clear();
  m_cellularPathFinder.apply(inputSegmentPairs,
                             WeightedNeighborhood<const CDCSegmentPair>(inputSegmentPairRelations),
                             m_segmentPairPaths);

  // Reduce to plain tracks
  for (const Path<const CDCSegmentPair>& segmentPairPath : m_segmentPairPaths) {
    outputTracks.push_back(CDCTrack::condense(segmentPairPath));
    for (const CDCSegmentPair* segmentPair : segmentPairPath) {
      segmentPair->getFromSegment()->getAutomatonCell().setTakenFlag();
      segmentPair->getToSegment()->getAutomatonCell().setTakenFlag();
    }
  }
}
