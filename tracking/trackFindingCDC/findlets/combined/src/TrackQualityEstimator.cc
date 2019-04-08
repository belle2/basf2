/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Eliachevitch                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/TrackQualityEstimator.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::ChooseableFilter<TrackQualityFilterFactory>;

TrackQualityEstimator::TrackQualityEstimator(const std::string& defaultFilterName)
  : m_trackQualityFilter(defaultFilterName)
{
  this->addProcessingSignalListener(&m_mcCloneLookUpFiller);
  this->addProcessingSignalListener(&m_trackQualityFilter);
}

std::string TrackQualityEstimator::getDescription()
{
  return "Set the quality indicator for CDC tracks and, if desired, delete tracks with a too low quality value.";
}

void TrackQualityEstimator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_trackQualityFilter.exposeParameters(moduleParamList, prefix);
  m_mcCloneLookUpFiller.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(prefixed(prefix, "deleteTracks"),
                                m_param_deleteTracks,
                                "Delete tracks below cut instead of just assigning quality indicator.",
                                m_param_deleteTracks);
}

void TrackQualityEstimator::apply(std::vector<CDCTrack>& tracks)
{

  if (m_trackQualityFilter.needsTruthInformation()) { m_mcCloneLookUpFiller.apply(tracks); }
  for (CDCTrack& track : tracks) {
    const double qualityIndicator = m_trackQualityFilter(track);
    track.setQualityIndicator(qualityIndicator);
  }

  if (m_param_deleteTracks) { // delete track with QI below cut threshold
    auto reject = [](const CDCTrack & track) {
      const double qualityIndicator = track.getQualityIndicator();
      return std::isnan(qualityIndicator);
    };
    erase_remove_if(tracks, reject);
  }
}
