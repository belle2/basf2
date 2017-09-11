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

  this->addProcessingSignalListener(&m_wireHitPreparer);
  this->addProcessingSignalListener(&m_clusterPreparer);
  this->addProcessingSignalListener(&m_segmentFinderFacetAutomaton);
  this->addProcessingSignalListener(&m_trackFinderSegmentPairAutomaton);
  this->addProcessingSignalListener(&m_trackFlightTimeAdjuster);
  this->addProcessingSignalListener(&m_trackExporter);

  ParamList paramList;
  const std::string prefix = "";
  this->exposeParams(&paramList, prefix);
  paramList.getParameter<std::string>("flightTimeEstimation").setDefaultValue("outwards");
  paramList.getParameter<std::string>("TrackOrientation").setDefaultValue("outwards");

  // Mimics earlier behaviour
  paramList.getParameter<bool>("WriteSegments").setDefaultValue(true);

  m_wireHits.reserve(1000);
  m_clusters.reserve(100);
  m_superClusters.reserve(100);
  m_segments.reserve(100);
  m_tracks.reserve(20);
}

std::string TrackFinderAutomaton::getDescription()
{
  return "Performs patter recognition in the CDC based on local hit following and application of a cellular automaton in two stages.";
}

void TrackFinderAutomaton::exposeParams(ParamList* paramList, const std::string& prefix)
{
  m_wireHitsSwapper.exposeParams(paramList, prefix);
  m_segmentsSwapper.exposeParams(paramList, prefix);
  m_tracksSwapper.exposeParams(paramList, prefix);

  m_wireHitPreparer.exposeParams(paramList, prefix);
  m_clusterPreparer.exposeParams(paramList, prefix);
  m_segmentFinderFacetAutomaton.exposeParams(paramList, prefix);
  m_trackFinderSegmentPairAutomaton.exposeParams(paramList, prefix);
  m_trackFlightTimeAdjuster.exposeParams(paramList, prefix);
  m_trackExporter.exposeParams(paramList, prefix);
}

void TrackFinderAutomaton::beginEvent()
{
  m_wireHits.clear();
  m_clusters.clear();
  m_superClusters.clear();
  m_segments.clear();
  m_tracks.clear();
  Super::beginEvent();
}

void TrackFinderAutomaton::apply()
{
  // Aquire the wire hits, segments and tracks from the DataStore in case they have already been created
  m_wireHitsSwapper.apply(m_wireHits);
  m_segmentsSwapper.apply(m_segments);
  m_tracksSwapper.apply(m_tracks);

  m_wireHitPreparer.apply(m_wireHits);
  m_clusterPreparer.apply(m_wireHits, m_clusters, m_superClusters);
  m_segmentFinderFacetAutomaton.apply(m_clusters, m_segments);
  m_trackFinderSegmentPairAutomaton.apply(m_segments, m_tracks);
  m_trackFlightTimeAdjuster.apply(m_tracks);
  m_trackExporter.apply(m_tracks);

  // Put the segments and tracks on the DataStore
  m_wireHitsSwapper.apply(m_wireHits);
  m_segmentsSwapper.apply(m_segments);
  m_tracksSwapper.apply(m_tracks);
}
