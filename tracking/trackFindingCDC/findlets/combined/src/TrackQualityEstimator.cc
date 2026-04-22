/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/TrackQualityEstimator.h>

#include <tracking/trackingUtilities/eventdata/tracks/CDCTrack.h>

#include <tracking/trackingUtilities/filters/base/ChooseableFilter.icc.h>

#include <tracking/trackingUtilities/utilities/StringManipulation.h>
#include <tracking/trackingUtilities/utilities/Algorithms.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <tracking/trackFindingCDC/filters/track/CDCTrackDeadBoardFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

template class TrackingUtilities::ChooseableFilter<TrackQualityFilterFactory>;

TrackQualityEstimator::TrackQualityEstimator(const std::string& defaultFilterName)
  : m_trackQualityFilter(defaultFilterName)
{
  this->addProcessingSignalListener(&m_mcCloneLookUpFiller);
  this->addProcessingSignalListener(&m_trackQualityFilter);
}

void TrackQualityEstimator::initialize()
{
  Super::initialize();
  // cache output of needsTruthInformation in member variable
  m_needsTruthInformation = m_trackQualityFilter.needsTruthInformation();
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

  moduleParamList->addParameter(prefixed(prefix, "resetTakenFlag"),
                                m_param_resetTakenFlag,
                                "Reset taken flag for deleted tracks so that hits can be used by subsequent TFs.",
                                m_param_resetTakenFlag);

  moduleParamList->addParameter(prefixed(prefix, "deactivateIfBadBoard"),
                                m_param_deactivateIfBadBoard,
                                "If true the filter will be deactivated in case a bad CDC board is detected at a position where a hole in the track is found.",
                                m_param_deactivateIfBadBoard);
}

void TrackQualityEstimator::apply(std::vector<CDCTrack>& tracks)
{

  if (m_needsTruthInformation) { m_mcCloneLookUpFiller.apply(tracks); }
  for (CDCTrack& track : tracks) {
    const double qualityIndicator = m_trackQualityFilter(track);
    track.setQualityIndicator(qualityIndicator);

    // check for dead boards in case of rejected tracks
    if (m_param_deactivateIfBadBoard && std::isnan(qualityIndicator)) {
      // set QI to infinity temporarily to prevent deletion if dead board found, will be set back to NAN later (after deletion step)
      if (cdcTrackDeadBoardFilter(track)) track.setQualityIndicator(std::numeric_limits<float>::infinity());
    }
  }


  if (m_param_deleteTracks) { // delete track with QI below cut threshold
    auto reject = [this](const CDCTrack & track) {
      const double qualityIndicator = track.getQualityIndicator();
      if (m_param_resetTakenFlag && std::isnan(qualityIndicator)) {
        track.forwardTakenFlag(false);
      }
      return std::isnan(qualityIndicator);
    };
    erase_remove_if(tracks, reject);
  }

  // reset NaN for the tracks where the filter has been deactivated
  for (auto& track : tracks) {
    if (std::isinf(track.getQualityIndicator())) track.setQualityIndicator(NAN);
  }

}
