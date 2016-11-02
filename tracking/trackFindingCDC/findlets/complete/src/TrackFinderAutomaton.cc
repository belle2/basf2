/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/complete/TrackFinderAutomaton.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFinderAutomaton::TrackFinderAutomaton()
{
  this->addProcessingSignalListener(&m_wireHitsSwapper);
  this->addProcessingSignalListener(&m_segmentsSwapper);
  this->addProcessingSignalListener(&m_tracksSwapper);

  this->addProcessingSignalListener(&m_wireHitTopologyPreparer);
  this->addProcessingSignalListener(&m_segmentFinderFacetAutomaton);
  this->addProcessingSignalListener(&m_trackFinderSegmentPairAutomaton);
  this->addProcessingSignalListener(&m_trackFlightTimeAdjuster);
  this->addProcessingSignalListener(&m_trackExporter);

  ModuleParamList moduleParamList;
  this->exposeParameters(&moduleParamList);
  moduleParamList.getParameter<std::string>("flightTimeEstimation").setDefaultValue("outwards");
  moduleParamList.getParameter<std::string>("SegmentOrientation").setDefaultValue("curling");
  moduleParamList.getParameter<std::string>("TrackOrientation").setDefaultValue("outwards");

  // Mimics earlier behaviour
  moduleParamList.getParameter<bool>("WriteSegments").setDefaultValue(true);
}

std::string TrackFinderAutomaton::getDescription()
{
  return "Performs patter recognition in the CDC based on local hit following and application of a cellular automaton in two stages.";
}

void TrackFinderAutomaton::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_wireHitsSwapper.exposeParameters(moduleParamList, prefix);
  m_segmentsSwapper.exposeParameters(moduleParamList, prefix);
  m_tracksSwapper.exposeParameters(moduleParamList, prefix);

  m_wireHitTopologyPreparer.exposeParameters(moduleParamList, prefix);
  m_segmentFinderFacetAutomaton.exposeParameters(moduleParamList, prefix);
  m_trackFinderSegmentPairAutomaton.exposeParameters(moduleParamList, prefix);
  m_trackFlightTimeAdjuster.exposeParameters(moduleParamList, prefix);
  m_trackExporter.exposeParameters(moduleParamList, prefix);
}

void TrackFinderAutomaton::apply()
{
  std::vector<CDCWireHit> wireHits;
  std::vector<CDCRecoSegment2D> segments;
  std::vector<CDCTrack> tracks;

  // Aquire the wire hits, segments and tracks from the DataStore
  m_wireHitsSwapper.apply(wireHits);
  m_segmentsSwapper.apply(segments);
  m_tracksSwapper.apply(tracks);

  wireHits.reserve(1000);
  segments.reserve(100);
  tracks.reserve(20);

  m_wireHitTopologyPreparer.apply(wireHits);
  m_segmentFinderFacetAutomaton.apply(wireHits, segments);
  m_trackFinderSegmentPairAutomaton.apply(segments, tracks);
  m_trackFlightTimeAdjuster.apply(tracks);
  m_trackExporter.apply(tracks);

  // Put the segments and tracks on the DataStore
  m_wireHitsSwapper.apply(wireHits);
  m_segmentsSwapper.apply(segments);
  m_tracksSwapper.apply(tracks);
}
