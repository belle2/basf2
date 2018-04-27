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
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackCreatorSingleSegments::TrackCreatorSingleSegments()
{
  m_param_minimalHitsBySuperLayerId = std::map<ISuperLayer, size_t> ({{0, 15}});
}

std::string TrackCreatorSingleSegments::getDescription()
{
  return "Creates a track for each segments that is yet unused by any of the given tracks.";
}

void TrackCreatorSingleSegments::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "MinimalHitsBySuperLayerId"),
                                m_param_minimalHitsBySuperLayerId,
                                "Map of super layer ids to minimum hit number, "
                                "for which left over segments shall be forwarded as tracks, "
                                "if the exceed the minimal hit requirement. Default empty.",
                                m_param_minimalHitsBySuperLayerId);
}

void TrackCreatorSingleSegments::apply(const std::vector<CDCSegment2D>& segments,
                                       std::vector<CDCTrack>& tracks)
{
  // Create tracks from left over segments
  // First figure out which segments do not share any hits with any of the given tracks
  // (if the tracks vector is empty this is the case for all segments)
  const bool toHits = true;
  for (const CDCSegment2D& segment : segments) {
    segment.unsetAndForwardMaskedFlag(toHits);
  }

  for (const CDCTrack& track : tracks) {
    track.setAndForwardMaskedFlag();
  }

  for (const CDCSegment2D& segment : segments) {
    segment.receiveMaskedFlag(toHits);
  }

  if (not m_param_minimalHitsBySuperLayerId.empty()) {
    for (const CDCSegment2D& segment : segments) {
      if (segment->hasMaskedFlag()) {
        int nMasked = 0;
        for (const CDCRecoHit2D& recoHit2D : segment) {
          if (recoHit2D.getWireHit()->hasMaskedFlag()) ++nMasked;
        }
        // Relaxed requirement of only 20% of hits masked by other tracks
        if (nMasked > segment.size() * 0.2) continue;
      }
      ISuperLayer iSuperLayer = segment.getISuperLayer();
      if (m_param_minimalHitsBySuperLayerId.count(iSuperLayer) and
          segment.size() >= m_param_minimalHitsBySuperLayerId[iSuperLayer]) {

        if (segment.getTrajectory2D().isFitted()) {
          tracks.push_back(CDCTrack(segment));
          segment.setAndForwardMaskedFlag(toHits);
          for (const CDCSegment2D& otherSegment : segments) {
            otherSegment.receiveMaskedFlag(toHits);
          }
        }
      }
    }
  }
}
