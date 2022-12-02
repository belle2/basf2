/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/eventTimeExtraction/findlets/TrackSelector.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void TrackSelector::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "minNumberCDCHits"), m_param_minNumberCDCHits,
                                "Minimal number of CDC hits for a track", m_param_minNumberCDCHits);
  moduleParamList->addParameter(prefixed(prefix, "maximumNumberOfTracks"), m_param_maximumNumberOfTracks,
                                "Maximal number of tracks in an event", m_param_maximumNumberOfTracks);
  moduleParamList->addParameter(prefixed(prefix, "minimumTrackPt"), m_param_minimumTrackPt,
                                "Minimal pt for a track", m_param_minimumTrackPt);
}

void TrackSelector::apply(std::vector<RecoTrack*>& tracks)
{
  const auto trackHasEnoughPtAndHits = [this](RecoTrack * rt) {
    return (rt->getNumberOfCDCHits() < m_param_minNumberCDCHits) or
           (rt->getMomentumSeed().Rho() < m_param_minimumTrackPt);
  };
  TrackFindingCDC::erase_remove_if(tracks, trackHasEnoughPtAndHits);

  // this lambda will sort in reverse order, meaning the tracks with the most CDC hits first
  const auto greaterHits = [](RecoTrack * lhs, RecoTrack * rhs) {
    return lhs->getNumberOfCDCHits() > rhs->getNumberOfCDCHits();
  };
  // sort by the amount of CDC hits
  std::sort(tracks.begin(), tracks.end(), greaterHits);

  const unsigned int maximalNumberOfElements = std::min(static_cast<unsigned int>(tracks.size()),
                                                        m_param_maximumNumberOfTracks);
  // limit to the maximum number of tracks
  tracks.resize(maximalNumberOfElements);

  B2DEBUG(25, "Limited number of selected tracks: " << tracks.size());
}
