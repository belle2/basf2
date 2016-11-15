/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSingleSegments.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string TrackCreatorSingleSegments::getDescription()
{
  return "Creates a track for each segments that is yet unused by any of the given tracks.";
}

void TrackCreatorSingleSegments::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "MinimalHitsForSingleSegmentTrackBySuperLayerId"),
                                m_param_minimalHitsForSingleSegmentTrackBySuperLayerId,
                                "Map of super layer ids to minimum hit number, "
                                "for which left over segments shall be forwarded as tracks, "
                                "if the exceed the minimal hit requirement. Default empty.",
                                m_param_minimalHitsForSingleSegmentTrackBySuperLayerId);
}

void TrackCreatorSingleSegments::apply(const std::vector<CDCSegment2D>& segments,
                                       std::vector<CDCTrack>& tracks)
{
  // Create tracks from left over segments
  // First figure out which segments do not share any hits with any of the given tracks
  // (if the tracks vector is empty this is the case for all segments)
  for (const CDCSegment2D& segment : segments) {
    segment.unsetAndForwardMaskedFlag();
  }

  for (const CDCTrack& track : tracks) {
    track.unsetAndForwardMaskedFlag();
  }

  for (const CDCSegment2D& segment : segments) {
    segment.receiveMaskedFlag();
  }

  if (not m_param_minimalHitsForSingleSegmentTrackBySuperLayerId.empty()) {
    for (const CDCSegment2D& segment : segments) {
      if (segment.getAutomatonCell().hasMaskedFlag()) continue;

      ISuperLayer iSuperLayer = segment.getISuperLayer();
      if (m_param_minimalHitsForSingleSegmentTrackBySuperLayerId.count(iSuperLayer) and
          segment.size() >= m_param_minimalHitsForSingleSegmentTrackBySuperLayerId[iSuperLayer]) {

        if (segment.getTrajectory2D().isFitted()) {
          tracks.push_back(CDCTrack(segment));
          segment.setAndForwardMaskedFlag();
          for (const CDCSegment2D& otherSegment : segments) {
            otherSegment.receiveMaskedFlag();
          }
        }
      }
    }
  }
}
