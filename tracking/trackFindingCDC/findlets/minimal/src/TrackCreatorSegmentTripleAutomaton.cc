/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSegmentTripleAutomaton.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/ca/Path.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string TrackCreatorSegmentTripleAutomaton::getDescription()
{
  return "Constructs tracks by extraction of segment triple paths in a cellular automaton.";
}

void TrackCreatorSegmentTripleAutomaton::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_cellularPathFinder.exposeParameters(moduleParamList, prefix);
}

void TrackCreatorSegmentTripleAutomaton::apply(const std::vector<CDCSegmentTriple>& inputSegmentTriples,
                                               const std::vector<WeightedRelation<const CDCSegmentTriple>>& inputSegmentTripleRelations,
                                               std::vector<CDCTrack>& outputTracks)
{
  // Obtain the segment triples as pointers
  std::vector<const CDCSegmentTriple*> segmentTriplePtrs =
    as_pointers<const CDCSegmentTriple>(inputSegmentTriples);

  m_segmentTriplePaths.clear();
  m_cellularPathFinder.apply(segmentTriplePtrs, inputSegmentTripleRelations, m_segmentTriplePaths);

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
