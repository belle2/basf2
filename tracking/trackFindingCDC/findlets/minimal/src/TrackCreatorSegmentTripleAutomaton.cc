/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSegmentTripleAutomaton.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>

#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>
#include <tracking/trackFindingCDC/ca/Path.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string TrackCreatorSegmentTripleAutomaton::getDescription()
{
  return "Constructs tracks by extraction of segment triple paths in a cellular automaton.";
}

void TrackCreatorSegmentTripleAutomaton::apply(const std::vector<CDCSegmentTriple>& inputSegmentTriples,
                                               const std::vector<WeightedRelation<const CDCSegmentTriple>>& inputSegmentTripleRelations,
                                               std::vector<CDCTrack>& outputTracks)
{
  m_segmentTriplePaths.clear();
  m_cellularPathFinder.apply(inputSegmentTriples,
                             WeightedNeighborhood<const CDCSegmentTriple>(inputSegmentTripleRelations),
                             m_segmentTriplePaths);

  // Reduce to plain tracks
  for (const Path<const CDCSegmentTriple>& segmentTriplePath : m_segmentTriplePaths) {
    outputTracks.push_back(CDCTrack::condense(segmentTriplePath));
    for (const CDCSegmentTriple* segmentTriple : segmentTriplePath) {
      segmentTriple->getStartSegment()->getAutomatonCell().setTakenFlag();
      segmentTriple->getMiddleSegment()->getAutomatonCell().setTakenFlag();
      segmentTriple->getEndSegment()->getAutomatonCell().setTakenFlag();
    }
  }
}
