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

#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/core/ModuleParam.h>

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

  ModuleParamList moduleParamList;
  const std::string prefix = "";
  this->exposeParameters(&moduleParamList, prefix);
  moduleParamList.getParameter<int>("SegmentPairRelationOnlyBest").setDefaultValue(1);

  m_segmentPairs.reserve(100);
  m_segmentPairRelations.reserve(100);
  m_preLinkingTracks.reserve(20);
  m_orientedTracks.reserve(20);
}

std::string TrackFinderSegmentPairAutomaton::getDescription()
{
  return "Generates tracks from segments using a cellular automaton built from segment pairs.";
}

void TrackFinderSegmentPairAutomaton::exposeParameters(ModuleParamList* moduleParamList,
                                                       const std::string& prefix)
{
  m_segmentPairCreator.exposeParameters(moduleParamList, prefixed(prefix, "SegmentPair"));
  m_segmentPairRelationCreator.exposeParameters(moduleParamList, prefixed(prefix, "SegmentPairRelation"));
  m_trackCreatorSegmentPairAutomaton.exposeParameters(moduleParamList, prefixed(prefix, "SegmentPairRelation"));
  m_trackCreatorSingleSegments.exposeParameters(moduleParamList, prefix);
  m_trackLinker.exposeParameters(moduleParamList, prefixed(prefix, "TrackRelation"));
  m_trackOrienter.exposeParameters(moduleParamList, prefix);
  m_segmentPairSwapper.exposeParameters(moduleParamList, prefix);
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

  std::vector<const CDCSegmentPair*> segmentPairPtrs =
    as_pointers<const CDCSegmentPair>(m_segmentPairs);
  m_segmentPairRelationCreator.apply(segmentPairPtrs, m_segmentPairRelations);

  m_trackCreatorSegmentPairAutomaton.apply(m_segmentPairs, m_segmentPairRelations, m_preLinkingTracks);

  m_trackCreatorSingleSegments.apply(inputSegments, m_preLinkingTracks);

  m_trackOrienter.apply(m_preLinkingTracks, m_orientedTracks);
  m_trackLinker.apply(m_orientedTracks, tracks);

  // Put the segment pairs on the DataStore
  m_segmentPairSwapper.apply(m_segmentPairs);
}
