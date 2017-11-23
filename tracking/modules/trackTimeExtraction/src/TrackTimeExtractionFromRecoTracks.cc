/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter, Thomas Hauth, Nils Braun                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackTimeExtraction/TrackTimeExtractionFromRecoTracks.h>
#include <tracking/timeExtraction/TimeExtractionUtils.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/trackFitting/fitter/base/TrackFitter.h>

#include <tracking/ckf/general/utilities/ClassMnemomics.h>

using namespace Belle2;
using namespace TrackFindingCDC;


/// Short description of the findlet
std::string TrackTimeExtractionFromRecoTracks::getDescription()
{
  return "Build the full covariance matrix for RecoTracks.";
}

TrackTimeExtractionFromRecoTracks::TrackTimeExtractionFromRecoTracks()
{
  addProcessingSignalListener(&m_recoTracksLoader);
  addProcessingSignalListener(&m_trackTimeExtraction);
}

void TrackTimeExtractionFromRecoTracks::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_recoTracksLoader.exposeParameters(moduleParamList, prefix);
  m_trackTimeExtraction.exposeParameters(moduleParamList, prefix);
}

void TrackTimeExtractionFromRecoTracks::apply()
{
  std::vector< RecoTrack const*> recoTracks;

  m_recoTracksLoader.apply(recoTracks);
  m_trackTimeExtraction.apply(recoTracks);
}

