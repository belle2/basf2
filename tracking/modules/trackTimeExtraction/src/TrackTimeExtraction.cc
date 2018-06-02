/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter, Thomas Hauth, Nils Braun                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackTimeExtraction/TrackTimeExtraction.h>
#include <tracking/timeExtraction/TimeExtractionUtils.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

#include <TRandom.h>

using namespace Belle2;
using namespace TrackFindingCDC;


/// Short description of the findlet
std::string TrackTimeExtraction::getDescription()
{
  return "Build the full covariance matrix for RecoTracks.";
}

TrackTimeExtraction::TrackTimeExtraction() : Super()
{
  ModuleParamList moduleParamList;
  const std::string prefix = "";
  this->exposeParameters(&moduleParamList, prefix);
}

void TrackTimeExtraction::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "maximalIterations"), m_param_maximalIterations,
                                "Maximal number of iterations to perform.",
                                m_param_maximalIterations);
  moduleParamList->addParameter(prefixed(prefix, "minimalIterations"), m_param_minimalIterations,
                                "Minimal number of iterations to perform.",
                                m_param_minimalIterations);

  moduleParamList->addParameter(prefixed(prefix, "minimalNumberCDCHits"), m_param_minimalNumberCDCHits,
                                "Minimal number of CDC hits track must contain to be used for extraction.",
                                m_param_minimalNumberCDCHits);

  moduleParamList->addParameter(prefixed(prefix, "minimalTimeDeviation"), m_param_minimalTimeDeviation,
                                "Minimal deviation between two extractions, to call the extraction as converged.",
                                m_param_minimalTimeDeviation);

  moduleParamList->addParameter(prefixed(prefix, "randomizeOnError"), m_param_randomizeOnError,
                                "Whether to randomize the extracted time, when the fit fails.",
                                m_param_randomizeOnError);
  moduleParamList->addParameter(prefixed(prefix, "randomizeLimits"), m_param_randomizeLimits,
                                "The maximal and minimal limit [-l, l] in which to randomize the extracted time on errors.",
                                m_param_randomizeLimits);

  moduleParamList->addParameter(prefixed(prefix, "overwriteExistingEstimation"), m_param_overwriteExistingEstimation,
                                "Whether to replace an existing time estimation or not.",
                                m_param_overwriteExistingEstimation);

  moduleParamList->addParameter(prefixed(prefix, "maximalExtractedT0"), m_param_maximalExtractedT0,
                                "Hard cut on this value of extracted times in the positive as well as the negative direction.",
                                m_param_maximalExtractedT0);

  moduleParamList->addParameter(prefixed(prefix, "t0Uncertainty"), m_param_t0Uncertainty, "Use this as sigma t0.",
                                m_param_t0Uncertainty);

  moduleParamList->addParameter(prefixed(prefix, "maximumTracksUsed"), m_param_maximumTracksUsed,
                                "the maximum amount of tracks used for the extraction.",
                                m_param_maximumTracksUsed);
  moduleParamList->addParameter(prefixed(prefix, "minimumTrackPt"), m_param_minimumTrackPt,
                                "Track Pt a track must at least have to be used for time fitting",
                                m_param_minimumTrackPt);
}

void TrackTimeExtraction::initialize()
{
  Super::initialize();
  m_eventT0.registerInDataStore();
}

void TrackTimeExtraction::apply(std::vector<RecoTrack*>& recoTracks)
{
  m_lastRunSucessful = false;

  if (not m_eventT0.isValid()) {
    m_eventT0.create();
  } else if (not m_param_overwriteExistingEstimation) {
    B2INFO("T0 estimation already present and overwriteExistingEstimation set to false. Skipping.");
    return;
  }

  auto selectedRecoTracks = TimeExtractionUtils::selectTracksForTimeExtraction(recoTracks, m_param_minimalNumberCDCHits,
                            m_param_maximumTracksUsed, m_param_minimumTrackPt);

  // check if there are any reco tracks at all available
  if (selectedRecoTracks.size() == 0) {
    B2DEBUG(25, "No tracks for time extraction satisfy the requirements, skipping this event ");
    return;
  }

  extractTrackTimeLoop(selectedRecoTracks);
}

void TrackTimeExtraction::extractTrackTimeLoop(std::vector<RecoTrack*>& recoTracks)
{
  TrackFitter trackFitter;

  // by default, we expect the time difference to the current t0 to be very small
  unsigned int loopCounter = 0;

  // store the previous best t0 extraction for CDC, if present
  std::tuple <bool, double, double> initialT0 = {false, 0.0f, 0.0f};
  if (m_eventT0->hasEventT0()) {
    initialT0 = {true, m_eventT0->getEventT0(), m_eventT0->getEventT0Uncertainty() };
  }

  for (; loopCounter < m_param_maximalIterations; loopCounter++) {

    B2DEBUG(25, "Using " << recoTracks.size() << " tracks for t0 fitting");
    for (RecoTrack* recoTrack : recoTracks) {
      recoTrack->deleteFittedInformation();
      trackFitter.fit(*recoTrack);
    }

    const double extractedTimeDelta = extractTrackTime(recoTracks);
    B2DEBUG(25, "Extracted Time delta in iteration " << loopCounter << " is " << extractedTimeDelta << " ns");
    if (std::isnan(extractedTimeDelta)) {
      B2ERROR("Extracted Time delta is NaN! Aborting.");
      break;
    } else {
      // either use an existing final value as start point, or use 0
      const double fullT0 = m_eventT0->hasEventT0() ? m_eventT0->getEventT0() : 0.0f;
      const double fullT0Updated = fullT0 + extractedTimeDelta;

      B2DEBUG(25, "Updating full event t0 to " << fullT0 << " (from previous EventT0) + " << extractedTimeDelta
              << " (from this iteration), total = " << fullT0Updated << " +- " << m_param_t0Uncertainty);

      m_eventT0->setEventT0(fullT0Updated, m_param_t0Uncertainty, Const::EDetector::CDC);

      // check for early exit criteria
      if (std::abs(extractedTimeDelta) < m_param_minimalTimeDeviation and loopCounter >= m_param_minimalIterations) {
        B2DEBUG(20, "Final delta T0 " << extractedTimeDelta
                << ". Needed " << loopCounter << " iterations.");
        m_lastRunSucessful = true;
        // The uncertainty was calculated using a test MC sample
        m_eventT0->addTemporaryEventT0(fullT0Updated, m_param_t0Uncertainty, Const::EDetector::CDC);
        break;
      }
    }
  }

  if (loopCounter == m_param_maximalIterations) {
    B2WARNING("Could not determine the track time in the maximum number of iterations to the needed precision.");
  }

  // set back the previously determined value, if one existed
  // this was modified by the iterations done during the finding procedure
  if (std::get<0>(initialT0)) {
    m_eventT0->setEventT0(std::get<1>(initialT0), std::get<1>(initialT0), Const::EDetector::CDC);
  } else {
    // otherwise, be sure to remove any values which might have been set during the finding
    // procedure
    m_eventT0->clearEventT0();
  }
}

bool TrackTimeExtraction::wasSucessful() const
{
  return m_lastRunSucessful;
}



double TrackTimeExtraction::extractTrackTime(std::vector<RecoTrack*>& recoTracks) const
{
  double sumFirstDerivatives = 0;
  double sumSecondDerivatives = 0;
  for (RecoTrack* recoTrack : recoTracks) {
    const auto& chi2Derivatives = TimeExtractionUtils::getChi2Derivatives(*recoTrack);
    const double& dchi2da = chi2Derivatives.first;
    const double& d2chi2da2 = chi2Derivatives.second;

    if (not std::isnan(dchi2da) and not std::isnan(d2chi2da2)) {
      if (d2chi2da2 > 20) {
        B2DEBUG(25, "Track with bad second derivative");
        continue;
      }
      sumFirstDerivatives += d2chi2da2;
      sumSecondDerivatives += dchi2da;
    }
  }

  double extractedDeltaT0 = 0;

  if (sumSecondDerivatives != 0 or sumFirstDerivatives != 0) {
    extractedDeltaT0 = sumSecondDerivatives / sumFirstDerivatives;
  } else {
    if (m_param_randomizeOnError) {
      extractedDeltaT0 = gRandom->Uniform(-m_param_minimalTimeDeviation,
                                          m_param_minimalTimeDeviation);
      B2WARNING("Strange results, setting randomly to " << extractedDeltaT0);
    }
  }

  if (std::abs(extractedDeltaT0) > m_param_maximalExtractedT0) {
    extractedDeltaT0 = std::copysign(m_param_maximalExtractedT0, extractedDeltaT0);
  }

  // is this really needed ?
  if (extractedDeltaT0 != 0) {
    for (RecoTrack* recoTrack : recoTracks) {
      ///recoTrack->setTimeSeed(recoTrack->getTimeSeed() + extractedT0);
      recoTrack->deleteFittedInformation();
    }
  }

  return extractedDeltaT0;
}
