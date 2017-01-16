/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/SegmentTrackAdderWithNormalization.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

/// Constructor for registering the sub.findlets
SegmentTrackAdderWithNormalization::SegmentTrackAdderWithNormalization() : Super()
{
  addProcessingSignalListener(&m_segmentTrackAdder);
  addProcessingSignalListener(&m_trackNormalizer);
}

/// Expose the parameters of the sub-findlets.
void SegmentTrackAdderWithNormalization::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_segmentTrackAdder.exposeParameters(moduleParamList, prefix);
  m_trackNormalizer.exposeParameters(moduleParamList, prefix);
}

/// Short description of the findlet
std::string SegmentTrackAdderWithNormalization::getDescription()
{
  return "Add the matched segments to the tracks and normalize the tracks afterwards. Also deletes all masked "
         "hits from the selection method before.";
}

/// Apply the findlet
void SegmentTrackAdderWithNormalization::apply(std::vector<WeightedRelation<CDCTrack, const CDCSegment2D>>& relations,
                                               std::vector<CDCTrack>& tracks)
{
  // Remove all masked hits from the track (which are double assigned)
  for (CDCTrack& track : tracks) {
    erase_remove_if(track, [](const CDCRecoHit3D & recoHit3D) {
      if (recoHit3D.getWireHit()->getAutomatonCell().hasMaskedFlag()) {
        recoHit3D.getWireHit()->getAutomatonCell().unsetTakenFlag();
        recoHit3D.getWireHit()->getAutomatonCell().unsetMaskedFlag();
        return true;
      } else {
        return false;
      }
    });
  }

  // Add all segments to the tracks, which were marked as matched
  m_segmentTrackAdder.apply(relations);

  // Remove all tracks that are (still) empty after the hit deletion and adding
  erase_remove_if(tracks, [](const CDCTrack & track) {
    return track.empty();
  });

  // Normalize the trajectory and hit contents of the tracks
  m_trackNormalizer.apply(tracks);
}