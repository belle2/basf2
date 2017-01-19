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
  addProcessingSignalListener(&m_sharedHitsMatcher);
  addProcessingSignalListener(&m_selectPairsWithSharedHits);
  addProcessingSignalListener(&m_chooseableSegmentTrackSelector);
  addProcessingSignalListener(&m_singleMatchSelector);
  addProcessingSignalListener(&m_segmentTrackAdderWithNormalization);
  addProcessingSignalListener(&m_trackRejecter);
}

std::string SegmentTrackCombinerFindlet::getDescription()
{
  return "Findlet for the combination of tracks and segments.";
}

void SegmentTrackCombinerFindlet::beginEvent()
{
  m_relations.clear();

  Super::beginEvent();
}

void SegmentTrackCombinerFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_sharedHitsMatcher.exposeParameters(moduleParamList, prefix);
  m_selectPairsWithSharedHits.exposeParameters(moduleParamList, prefixed("sharedHits", prefix));
  m_chooseableSegmentTrackSelector.exposeParameters(moduleParamList, prefixed("segmentTrack", prefix));
  m_singleMatchSelector.exposeParameters(moduleParamList, prefix);
  m_segmentTrackAdderWithNormalization.exposeParameters(moduleParamList, prefix);
  m_trackRejecter.exposeParameters(moduleParamList, prefixed(prefix, "track"));
}

// Do the combination work. See the SegmentTrackCombiner methods for full details.
void SegmentTrackCombinerFindlet::apply(std::vector<TrackFindingCDC::CDCSegment2D>& segments,
                                        std::vector<TrackFindingCDC::CDCTrack>& tracks)
{
  m_trackNormalizer.apply(tracks);

  // Add a precut to add segments which are fully taken immediately at this stage
  for (const CDCSegment2D& segment : segments) {
    if (segment.isFullyTaken()) {
      segment.getAutomatonCell().setTakenFlag();
    } else {
      segment.getAutomatonCell().unsetTakenFlag();
    }
  }

  // After that, relations contains all pairs of segments and tracks, with the number of shared hits as weight
  m_sharedHitsMatcher.apply(tracks, segments, m_relations);

  // Require a certain (definable) amount of shared hits between segments and tracks
  m_selectPairsWithSharedHits.apply(m_relations);

  // Apply a (mva) filter to all combinations
  m_chooseableSegmentTrackSelector.apply(m_relations);

  // Search for the best combinations
  m_singleMatchSelector.apply(m_relations);

  // Add those combinations and remove all hits, that are part of other tracks (non-selected combinations)
  m_segmentTrackAdderWithNormalization.apply(m_relations, tracks, segments);

  // Reject tracks according to a (mva) filter
  m_trackRejecter.apply(tracks);
}
