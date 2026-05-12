/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/general/findlets/TrackFitterAndDeleter.h>
#include <tracking/trackingUtilities/utilities/Algorithms.h>
#include <tracking/trackingUtilities/utilities/StringManipulation.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;

void TrackFitterAndDeleter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "trackFitType"),
                                m_trackFitType, "Type of track fit algorithm to use the corresponding DAFParameter, the list is defined in DAFConfiguration class.",
                                m_trackFitType);
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
  TrackingUtilities::erase_remove_if(recoTracks, trackWasNotFitted);
}
