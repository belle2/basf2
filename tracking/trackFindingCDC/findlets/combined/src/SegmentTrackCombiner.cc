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
  addProcessingSignalListener(&m_bestMatchSelector);
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

  m_trackNormalizer.exposeParameters(moduleParamList, prefix);
  m_sharedHitsMatcher.exposeParameters(moduleParamList, prefix);
  m_selectPairsWithSharedHits.exposeParameters(moduleParamList, prefixed("sharedHits", prefix));
  m_chooseableSegmentTrackSelector.exposeParameters(moduleParamList, prefixed("segmentTrack", prefix));
  m_bestMatchSelector.exposeParameters(moduleParamList, prefix);
  m_segmentTrackAdderWithNormalization.exposeParameters(moduleParamList, prefix);
  m_trackRejecter.exposeParameters(moduleParamList, prefixed(prefix, "track"));

  moduleParamList->getParameter<double>("sharedHitsCutValue").setDefaultValue(1.0);
  moduleParamList->getParameter<bool>("useOnlySingleBestCandidate").setDefaultValue(false);
}

// Do the combination work. See the SegmentTrackCombiner methods for full details.
void SegmentTrackCombinerFindlet::apply(std::vector<TrackFindingCDC::CDCSegment2D>& segments,
                                        std::vector<TrackFindingCDC::CDCTrack>& tracks)
{
  for (const CDCTrack& track : tracks) {
    track.unsetAndForwardMaskedFlag();
  }

  m_trackNormalizer.apply(tracks);

  // TODO: Add segments which are fully taken

  // After that, relations contains all pairs of segments and tracks, with the number of shared hits as weight
  m_sharedHitsMatcher.apply(tracks, segments, m_relations);

  // Require a certain (definable) amount of shared hits between segments and tracks
  m_selectPairsWithSharedHits.apply(m_relations);

  // Apply a (mva) filter to all combinations
  m_chooseableSegmentTrackSelector.apply(m_relations);

  // Search for the best combination if there is more than one possibility and remove the hits from all other ones
  m_bestMatchSelector.apply(m_relations);

  // Add the remaining combinations
  m_segmentTrackAdderWithNormalization.apply(m_relations, tracks);

  // Reject tracks according to a (mva) filter
  m_trackRejecter.apply(tracks);
}
