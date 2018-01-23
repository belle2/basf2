/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/TrackFinderSegmentTripleAutomaton.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFinderSegmentTripleAutomaton::TrackFinderSegmentTripleAutomaton()
{
  this->addProcessingSignalListener(&m_axialSegmentPairCreator);
  this->addProcessingSignalListener(&m_segmentTripleCreator);
  this->addProcessingSignalListener(&m_segmentTripleRelationCreator);
  this->addProcessingSignalListener(&m_trackCreatorSegmentTripleAutomaton);
  this->addProcessingSignalListener(&m_trackCreatorSingleSegments);
  this->addProcessingSignalListener(&m_trackLinker);
  this->addProcessingSignalListener(&m_trackOrienter);
  this->addProcessingSignalListener(&m_segmentTripleSwapper);

  m_axialSegmentPairs.reserve(75);
  m_segmentTriples.reserve(100);
  m_segmentTripleRelations.reserve(100);
  m_preLinkingTracks.reserve(20);
  m_orientedTracks.reserve(20);
}

std::string TrackFinderSegmentTripleAutomaton::getDescription()
{
  return "Generates tracks from segments using a cellular automaton built from segment triples.";
}

void TrackFinderSegmentTripleAutomaton::exposeParameters(ModuleParamList* moduleParamList,
                                                         const std::string& prefix)
{
  m_axialSegmentPairCreator.exposeParameters(moduleParamList, prefixed(prefix, "axialSegmentPair"));
  m_segmentTripleCreator.exposeParameters(moduleParamList, prefixed(prefix, "segmentTriple"));
  m_segmentTripleRelationCreator.exposeParameters(moduleParamList, prefixed(prefix, "segmentTripleRelation"));
  m_trackCreatorSegmentTripleAutomaton.exposeParameters(moduleParamList, prefixed(prefix, "segmentTripleRelation"));
  m_trackCreatorSingleSegments.exposeParameters(moduleParamList, prefix);
  m_trackLinker.exposeParameters(moduleParamList, prefixed(prefix, "TrackRelation"));
  m_trackOrienter.exposeParameters(moduleParamList, prefix);
  m_segmentTripleSwapper.exposeParameters(moduleParamList, prefix);
}

void TrackFinderSegmentTripleAutomaton::beginEvent()
{
  m_axialSegmentPairs.clear();
  m_segmentTriples.clear();
  m_segmentTripleRelations.clear();
  m_preLinkingTracks.clear();
  m_orientedTracks.clear();
  Super::beginEvent();
}

void TrackFinderSegmentTripleAutomaton::apply(const std::vector<CDCSegment2D>& inputSegments,
                                              std::vector<CDCTrack>& tracks)
{
  m_axialSegmentPairCreator.apply(inputSegments, m_axialSegmentPairs);

  m_segmentTripleCreator.apply(inputSegments, m_axialSegmentPairs, m_segmentTriples);

  std::vector<const CDCSegmentTriple*> segmentTriplePtrs =
    as_pointers<const CDCSegmentTriple>(m_segmentTriples);
  m_segmentTripleRelationCreator.apply(segmentTriplePtrs, m_segmentTripleRelations);

  m_trackCreatorSegmentTripleAutomaton.apply(m_segmentTriples, m_segmentTripleRelations, m_preLinkingTracks);

  m_trackCreatorSingleSegments.apply(inputSegments, m_preLinkingTracks);
  m_trackOrienter.apply(m_preLinkingTracks, m_orientedTracks);
  m_trackLinker.apply(m_orientedTracks, tracks);

  // Put the segment triples on the DataStore
  m_segmentTripleSwapper.apply(m_segmentTriples);
}
