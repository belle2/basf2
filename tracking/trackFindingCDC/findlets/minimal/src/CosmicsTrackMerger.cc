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
#include <tracking/trackFindingCDC/numerics/Angle.h>
#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CosmicsTrackMerger::CosmicsTrackMerger()
{
  addProcessingSignalListener(&m_trackRelationFilter);
}

std::string CosmicsTrackMerger::getDescription()
{
  return "Merge cosmics tracks based on different filter criteria. "
         "It assumes that there is only cosmics one track in the event.";
}

void CosmicsTrackMerger::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_trackRelationFilter.exposeParameters(moduleParamList, prefix);
}

void CosmicsTrackMerger::apply(const std::vector<CDCTrack>& inputTracks, std::vector<CDCTrack>& outputTracks)
{
  // Some simple cases
  if (inputTracks.empty()) {
    return;
  } else if (inputTracks.size() == 1) {
    outputTracks.push_back(inputTracks.front());
    return;
  }

  for (const CDCTrack& cdcTrack : inputTracks) {
    cdcTrack.getAutomatonCell().unsetMaskedFlag();
  }

  // Create linking relations
  m_trackRelations.clear();

  for (auto fromIt = inputTracks.begin(); fromIt != inputTracks.end(); fromIt++) {
    for (auto toIt = fromIt; toIt != inputTracks.end(); toIt++) {
      if (fromIt != toIt) {
        const CDCTrack& from = *fromIt;
        const CDCTrack& to = *toIt;
        Weight weight = m_trackRelationFilter(Relation<const CDCTrack>(&from, &to));
        if (not std::isnan(weight)) {
          m_trackRelations.emplace_back(&from, weight, &to);
        }
      }
    }
  }

  // TODO: see how we can generalize this
  const auto& trackSorter = [](const CDCTrack * lhs, const CDCTrack * rhs) {
    return lhs->getStartRecoPos3D().y() > rhs->getStartRecoPos3D().y();
  };

  if (not m_trackRelations.empty()) {
    std::sort(m_trackRelations.begin(), m_trackRelations.end());

    for (auto& relation : m_trackRelations) {
      if (relation.getFrom()->getAutomatonCell().hasMaskedFlag() or relation.getTo()->getAutomatonCell().hasMaskedFlag()) {
        continue;
      }

      // Create a list of valid cosmic tracks
      std::vector<const CDCTrack*> outputTrackPath = {relation.getFrom(), relation.getTo()};

      // Sort the tracks by their start position from top to bottom
      std::sort(outputTrackPath.begin(), outputTrackPath.end(), trackSorter);

      // As we assume only one track per event, we merge them all together.
      outputTracks.push_back(CDCTrack::condense(outputTrackPath));

      relation.getFrom()->getAutomatonCell().setMaskedFlag();
      relation.getTo()->getAutomatonCell().setMaskedFlag();
    }
  }

  // also add the rest of the tracks
  for (const CDCTrack& track : inputTracks) {
    if (not track.getAutomatonCell().hasMaskedFlag()) {
      outputTracks.push_back(track);
    }
  }

  for (const CDCTrack& cdcTrack : inputTracks) {
    cdcTrack.getAutomatonCell().unsetMaskedFlag();
  }
}
