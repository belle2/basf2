/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/CosmicsTrackMerger.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CosmicsTrackMerger::CosmicsTrackMerger()
{
}

std::string CosmicsTrackMerger::getDescription()
{
  return "Merge cosmics tracks based on different filter criteria. "
         "It assumes that there is only cosmics one track in the event.";
}

void CosmicsTrackMerger::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter("useSimpleApproach", m_param_useSimpleApproach,
                                "Use a simple approach just based on the number of tracks in the event",
                                m_param_useSimpleApproach);

  moduleParamList->addParameter("minimalNumberOfHits", m_param_minimalNumberOfHits,
                                "Minimal amount of hits a track must have to be used in the merging procedure.",
                                m_param_minimalNumberOfHits);
}

void CosmicsTrackMerger::apply(const std::vector<CDCTrack>& inputTracks, std::vector<CDCTrack>& outputTracks)
{
  // Create a list of valid cosmic tracks sorted by their y position
  std::vector<const CDCTrack*> outputTrackPath;
  outputTrackPath.reserve(inputTracks.size());

  const auto& fulfillsFeasibleCriteria = [this](const CDCTrack & track) {
    return track.size() >= m_param_minimalNumberOfHits;
  };

  for (const CDCTrack& track : inputTracks) {
    if (fulfillsFeasibleCriteria(track)) {
      outputTrackPath.push_back(&track);
    }
  }

  // Sort the tracks by their start position from top to bottom
  // TODO: see how we can generalize this
  const auto& trackSorter = [](const CDCTrack * lhs, const CDCTrack * rhs) {
    return lhs->getStartRecoPos3D().y() > rhs->getStartRecoPos3D().y();
  };
  std::sort(outputTrackPath.begin(), outputTrackPath.end(), trackSorter);

  if (m_param_useSimpleApproach) {
    if (outputTrackPath.size() > 2) {
      B2WARNING("Having found more than 2 possible candidates. The simple approach does not work here.");
      outputTrackPath.clear();
    }
  } else {
    B2FATAL("Not implemented!");
  }

  if (not outputTrackPath.empty()) {
    // As we assume only one track per event, we merge them all together.
    outputTracks.push_back(CDCTrack::condense(outputTrackPath));
  }
}
