/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/general/findlets/TrackFitterAndDeleter.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;

void TrackFitterAndDeleter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "trackFitType"),
                                m_trackFitType, "The DAF configuration to use.", m_trackFitType);
}

void TrackFitterAndDeleter::apply(std::vector<RecoTrack*>& recoTracks)
{
  TrackFitter trackFitter((DAFConfiguration::ETrackFitType)m_trackFitType);
  for (RecoTrack* recoTrack : recoTracks) {
    trackFitter.fit(*recoTrack);
  }

  // Remove all non-fitted tracks
  const auto trackWasNotFitted = [](RecoTrack * recoTrack) {
    return not recoTrack->wasFitSuccessful();
  };
  TrackFindingCDC::erase_remove_if(recoTracks, trackWasNotFitted);
}
