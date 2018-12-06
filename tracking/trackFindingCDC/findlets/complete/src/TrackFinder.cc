/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/complete/TrackFinder.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/core/ModuleParam.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFinder::TrackFinder()
{
  this->addProcessingSignalListener(&m_wireHitPreparer);
  this->addProcessingSignalListener(&m_clusterPreparer);
  this->addProcessingSignalListener(&m_segmentFinderFacetAutomaton);

  this->addProcessingSignalListener(&m_axialTrackFinderLegendre);
  this->addProcessingSignalListener(&m_trackQualityAsserter);

  this->addProcessingSignalListener(&m_stereoHitFinder);
  this->addProcessingSignalListener(&m_segmentTrackCombiner);

  this->addProcessingSignalListener(&m_trackFinderSegmentPairAutomaton);
  this->addProcessingSignalListener(&m_trackCombiner);

  this->addProcessingSignalListener(&m_finalTrackQualityAsserter);
  this->addProcessingSignalListener(&m_trackCreatorSingleSegments);
  this->addProcessingSignalListener(&m_trackExporter);

  const std::string prefix = "";
  // Setup m_wireHitPreparer
  {
    ModuleParamList moduleParamList;
    m_wireHitPreparer.exposeParameters(&moduleParamList, prefix);

    moduleParamList.getParameter<std::string>("flightTimeEstimation").setDefaultValue("outwards");
  }

  // Setup m_trackQualityAsserter
  {
    ModuleParamList moduleParamList;
    m_trackQualityAsserter.exposeParameters(&moduleParamList, prefix);

    std::vector<std::string> corrections({"B2B"});
    moduleParamList.getParameter<std::vector<std::string>>("corrections")
                                                        .setDefaultValue(corrections);
  }

  // Setup m_finalTrackQualityAsserter
  {
    ModuleParamList moduleParamList;
    m_finalTrackQualityAsserter.exposeParameters(&moduleParamList, prefix);

    std::vector<std::string> corrections({"LayerBreak", "OneSuperlayer", "Small"});
    moduleParamList.getParameter<std::vector<std::string>>("corrections")
                                                        .setDefaultValue(corrections);
  }
}

std::string TrackFinder::getDescription()
{
  return "Combined track finder using the global legendre finder as well as the cellular automaton "
         "track finder.";
}

void TrackFinder::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  // Expose a selection of the parameters
  m_wireHitPreparer.exposeParameters(moduleParamList, prefix);
  m_trackExporter.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter("withCA",
                                m_param_withCA,
                                "Also run the segment linking track finder and combine results.",
                                m_param_withCA);
}

void TrackFinder::apply()
{
  // Reserve some space for the created objects.
  std::vector<CDCWireHit> m_wireHits;
  std::vector<CDCWireHitCluster> m_clusters;
  std::vector<CDCWireHitCluster> m_superClusters;
  std::vector<CDCSegment2D> m_segments;
  std::vector<CDCTrack> m_axialTracks;
  std::vector<CDCTrack> m_tracks;
  m_wireHits.reserve(2000);
  m_clusters.reserve(200);
  m_superClusters.reserve(100);
  m_segments.reserve(200);
  m_axialTracks.reserve(30);
  m_tracks.reserve(30);

  m_wireHitPreparer.apply(m_wireHits);
  m_clusterPreparer.apply(m_wireHits, m_clusters, m_superClusters);
  m_segmentFinderFacetAutomaton.apply(m_clusters, m_segments);

  m_axialTrackFinderLegendre.apply(m_wireHits, m_axialTracks);
  m_trackQualityAsserter.apply(m_axialTracks);

  m_stereoHitFinder.apply(m_wireHits, m_axialTracks);
  m_segmentTrackCombiner.apply(m_segments, m_axialTracks);

  if (m_param_withCA) {
    m_trackFinderSegmentPairAutomaton.apply(m_segments, m_tracks);
    m_trackCombiner.apply(m_axialTracks, m_tracks, m_tracks);
  } else {
    m_tracks.swap(m_axialTracks);
  }

  m_finalTrackQualityAsserter.apply(m_tracks);

  if (m_param_withCA) {
    m_trackCreatorSingleSegments.apply(m_segments, m_tracks);
  }

  m_trackExporter.apply(m_tracks);
}
