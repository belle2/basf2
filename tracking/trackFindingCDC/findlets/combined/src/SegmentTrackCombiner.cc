/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/SegmentTrackCombiner.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

SegmentTrackCombinerFindlet::SegmentTrackCombinerFindlet()
{
  addProcessingSignalListener(&m_trackNormalizer);
  addProcessingSignalListener(&m_trackRejecter);
  addProcessingSignalListener(&m_chooseableSegmentTrackFilter);
  //addProcessingSignalListener(m_combiner);
}

std::string SegmentTrackCombinerFindlet::getDescription()
{
  return "Findlet for the combination of tracks and segments.";
}

void SegmentTrackCombinerFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_trackNormalizer.exposeParameters(moduleParamList, prefix);
  m_trackRejecter.exposeParameters(moduleParamList, prefixed(prefix, "track"));
  m_chooseableSegmentTrackFilter.exposeParameters(moduleParamList, prefixed("segmentTrack", prefix));
  //m_combiner.exposeParameters(moduleParamList, prefix);
}

// Do the combination work. See the SegmentTrackCombiner methods for full details.
void SegmentTrackCombinerFindlet::apply(std::vector<TrackFindingCDC::CDCSegment2D>& segments,
                                        std::vector<TrackFindingCDC::CDCTrack>& tracks)
{
  m_trackNormalizer.apply(tracks);

  m_combiner.match(m_chooseableSegmentTrackFilter, tracks, segments);

  m_trackRejecter.apply(tracks);

  m_trackNormalizer.apply(tracks);
}
