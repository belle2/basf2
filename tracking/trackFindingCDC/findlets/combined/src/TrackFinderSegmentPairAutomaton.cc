/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/TrackFinderSegmentPairAutomaton.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFinderSegmentPairAutomaton::TrackFinderSegmentPairAutomaton()
{
  this->addProcessingSignalListener(&m_segmentPairCreator);
  this->addProcessingSignalListener(&m_segmentPairRelationCreator);
  this->addProcessingSignalListener(&m_trackCreatorSegmentPairAutomaton);
  this->addProcessingSignalListener(&m_trackCreatorSingleSegments);
  this->addProcessingSignalListener(&m_trackLinker);
  this->addProcessingSignalListener(&m_trackOrienter);
  this->addProcessingSignalListener(&m_segmentPairSwapper);

  ParamList paramList;
  const std::string prefix = "";
  this->exposeParams(&paramList, prefix);
  paramList.getParameter<int>("SegmentPairRelationOnlyBest").setDefaultValue(1);

  m_segmentPairs.reserve(100);
  m_segmentPairRelations.reserve(100);
  m_preLinkingTracks.reserve(20);
  m_orientedTracks.reserve(20);
}

std::string TrackFinderSegmentPairAutomaton::getDescription()
{
  return "Generates tracks from segments using a cellular automaton built from segment pairs.";
}

void TrackFinderSegmentPairAutomaton::exposeParams(ParamList* paramList,
                                                   const std::string& prefix)
{
  m_segmentPairCreator.exposeParams(paramList, prefixed(prefix, "SegmentPair"));
  m_segmentPairRelationCreator.exposeParams(paramList, prefixed(prefix, "SegmentPairRelation"));
  m_trackCreatorSegmentPairAutomaton.exposeParams(paramList, prefix);
  m_trackCreatorSingleSegments.exposeParams(paramList, prefix);
  m_trackLinker.exposeParams(paramList, prefixed(prefix, "TrackRelation"));
  m_trackOrienter.exposeParams(paramList, prefix);
  m_segmentPairSwapper.exposeParams(paramList, prefix);
}

void TrackFinderSegmentPairAutomaton::beginEvent()
{
  m_segmentPairs.clear();
  m_segmentPairRelations.clear();
  m_preLinkingTracks.clear();
  m_orientedTracks.clear();
  Super::beginEvent();
}

void TrackFinderSegmentPairAutomaton::apply(const std::vector<CDCSegment2D>& inputSegments,
                                            std::vector<CDCTrack>& tracks)
{
  m_segmentPairCreator.apply(inputSegments, m_segmentPairs);
  m_segmentPairRelationCreator.apply(m_segmentPairs, m_segmentPairRelations);
  m_trackCreatorSegmentPairAutomaton.apply(m_segmentPairs, m_segmentPairRelations, m_preLinkingTracks);

  m_trackCreatorSingleSegments.apply(inputSegments, m_preLinkingTracks);

  m_trackOrienter.apply(m_preLinkingTracks, m_orientedTracks);
  m_trackLinker.apply(m_orientedTracks, tracks);

  // Put the segment pairs on the DataStore
  m_segmentPairSwapper.apply(m_segmentPairs);
}
