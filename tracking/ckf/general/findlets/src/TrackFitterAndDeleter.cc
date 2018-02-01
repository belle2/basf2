/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/general/findlets/TrackFitterAndDeleter.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;

void TrackFitterAndDeleter::apply(std::vector<RecoTrack*>& recoTracks)
{
  for (RecoTrack* recoTrack : recoTracks) {
    m_trackFitter->fit(*recoTrack);
  }

  // Remove all non-fitted tracks
  const auto trackWasNotFitted = [](RecoTrack * recoTrack) {
    return not recoTrack->wasFitSuccessful();
  };
  TrackFindingCDC::erase_remove_if(recoTracks, trackWasNotFitted);
}

void TrackFitterAndDeleter::beginRun()
{
  m_trackFitter = std::make_unique<Belle2::TrackFitter>();
}
