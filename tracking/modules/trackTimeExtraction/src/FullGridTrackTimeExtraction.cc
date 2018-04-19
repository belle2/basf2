/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter, Thomas Hauth, Nils Braun                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackTimeExtraction/FullGridTrackTimeExtraction.h>
#include <tracking/timeExtraction/TimeExtractionUtils.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

#include <numeric>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {

  /// Helper function to map a function over a list and sum the results for double types
  template<class T, class AFunction>
  double sum(std::vector<T>& list, const AFunction& function)
  {
    return std::accumulate(std::begin(list), std::end(list), 0.0, [&function](double value, T & item) {
      return value + function(item);
    });
  }

  /// Helper function to map a function over a list and sum the results for std::pair types
  template<class T, class AFunction>
  std::pair<double, double> sumPaired(std::vector<T>& list, const AFunction& function)
  {
    return std::accumulate(std::begin(list), std::end(list), std::make_pair(0.0, 0.0), [&function](std::pair<double, double>& value,
    T & item) {
      const auto& result = function(item);
      value.first += result.first;
      value.second += result.second;
      return value;
    });
  }
}
/// Extract the first and second chi^2 derivatives from each fittable track and build the mean.
std::pair<double, double> FullGridTrackTimeExtraction::extractChi2DerivativesHelper(std::vector<RecoTrack*>& recoTracks,
    std::map<RecoTrack*, bool>& fittableRecoTracks)
{
  unsigned int numberOfFittableRecoTracks = 0;
  auto chi2Derivates = sumPaired(recoTracks, [&](RecoTrack * recoTrack) {
    if (fittableRecoTracks[recoTrack]) {
      numberOfFittableRecoTracks++;
      return TimeExtractionUtils::getChi2Derivatives(*recoTrack);
    } else {
      return std::make_pair(0.0, 0.0);
    }
  });

  chi2Derivates.first /= numberOfFittableRecoTracks;
  chi2Derivates.second /= numberOfFittableRecoTracks;

  return chi2Derivates;
}

/// Helper function to add the "value" to the reco track time seeds and fit them
double FullGridTrackTimeExtraction::setTimeAndFitTracks(double value,
                                                        std::vector<RecoTrack*>& recoTracks,
                                                        std::map<RecoTrack*, bool>& fittableRecoTracks)
{
  TrackFitter trackFitter;
  m_eventT0->setEventT0(value, m_param_t0Uncertainty, Const::EDetector::CDC);

  double summedChi2 = 0;
  unsigned int numberOfFittableRecoTracks = 0;

  for (const auto& recoTrack : recoTracks) {
    recoTrack->deleteFittedInformation();
    trackFitter.fit(*recoTrack);

    B2DEBUG(50, "Fit of RecoTrack was successful = "  << recoTrack->wasFitSuccessful());

    if (not recoTrack->wasFitSuccessful()) {
      fittableRecoTracks[recoTrack] = false;
    } else {
      numberOfFittableRecoTracks++;
      summedChi2 += TimeExtractionUtils::extractReducedChi2(*recoTrack);
    }
  }

  return summedChi2 / numberOfFittableRecoTracks;
}

/**
 * Main function used in the module: Repeat the time extraction of the reco tracks starting from the given start
 * value until:
 * (a) the maximum number of steps is reached
 * (b) the extracted time is not in the range [t0 min, t0 max]
 * (c) the result is non-sense (e.g. NaN)
 *
 * Each extracted time is either added to the tries or the converged tries list, depending on if the chi^2 is small
 * and d^2 chi^2 / (d alpha)^2 is large enough.
 *
 * In the end, the tracks are reset to have the initial time seed.
 */
void FullGridTrackTimeExtraction::extractTrackTimeFrom(std::vector<RecoTrack*>& recoTracks, const double& startValue,
                                                       const unsigned int steps,
                                                       std::vector<T0Try>& tries, std::vector<T0Try>& convergedTries,
                                                       const double& minimalT0, const double& maximalT0)
{
  // Store which reco tracks we should use (and where the fit already failed)
  std::map<RecoTrack*, bool> fittableRecoTracks;

  for (RecoTrack* recoTrack : recoTracks) {
    fittableRecoTracks[recoTrack] = true;
  }

  // Store the first try ( = start value) with the chi^2
  B2DEBUG(50, "Trying t0 value of " << startValue << " ns");
  const double firstChi2 = setTimeAndFitTracks(startValue, recoTracks, fittableRecoTracks);

  if (std::isnan(firstChi2)) {
    B2DEBUG(50, "No fittable RecoTracks, bailing out");
    return;
  }

  tries.emplace_back(startValue, firstChi2);

  double extracted_time = startValue;

  for (unsigned int i = 0; i < steps; i++) {
    // Extract the time for the next time step
    std::pair<double, double> extractedDerivativePair = extractChi2DerivativesHelper(recoTracks, fittableRecoTracks);
    extracted_time += extractedDerivativePair.first / extractedDerivativePair.second;

    if (extracted_time > maximalT0 or extracted_time < minimalT0 or std::isnan(extracted_time)) {
      break;
    }

    B2DEBUG(50, "Upadting to t0 time to " << extracted_time);
    // Apply this new extracted time and extract the chi^2
    const double chi2 = setTimeAndFitTracks(extracted_time, recoTracks, fittableRecoTracks);

    if (std::isnan(chi2)) {
      B2DEBUG(50, "No fittable RecoTracks, bailing out");
      break;
    }

    if (chi2 > 10) {
      B2DEBUG(50, "Chi2 larger than 10, bailing out");
      break;
    }

    // Decide if we are already finished or not depending on the extracted values.
    const bool finished = extractedDerivativePair.second > 2.7122 and chi2 < 1.739;

    if (finished) {
      B2DEBUG(50, "Fit with t0 " << extracted_time << "converged with chi2 " << extracted_time);
      convergedTries.emplace_back(extracted_time, chi2);
      break;
    }

    B2DEBUG(50, "Fit with t0 " << extracted_time << "not converged with chi2 " << extracted_time);
    tries.emplace_back(extracted_time, chi2);
  }

  // Reset all RecoTracks
  for (const auto& recoTrack : recoTracks) {
    recoTrack->deleteFittedInformation();
  }
}


void FullGridTrackTimeExtraction::initialize()
{
  Super::initialize();

  m_eventT0.registerInDataStore();
}


FullGridTrackTimeExtraction::FullGridTrackTimeExtraction() : Super()
{
  ModuleParamList moduleParamList;
  const std::string prefix = "";
  this->exposeParameters(&moduleParamList, prefix);
}

void FullGridTrackTimeExtraction::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "minimalNumberCDCHits"), m_param_minimalNumberCDCHits,
                                "Minimal number of CDC hits track must contain to be used for extraction.",
                                m_param_minimalNumberCDCHits);

  moduleParamList->addParameter(prefixed(prefix, "minimalT0Shift"), m_param_minimalT0Shift,
                                "Minimal shift of the event time which is allowed.",
                                m_param_minimalT0Shift);

  moduleParamList->addParameter(prefixed(prefix, "maximalT0Shift"), m_param_maximalT0Shift,
                                "Maximal shift of the event time which is allowed.",
                                m_param_maximalT0Shift);

  moduleParamList->addParameter(prefixed(prefix, "numberOfGrids"), m_param_numberOfGrids, "Number of shifts to try out.",
                                m_param_numberOfGrids);

  moduleParamList->addParameter(prefixed(prefix, "t0Uncertainty"), m_param_t0Uncertainty, "Use this as sigma t0.",
                                m_param_t0Uncertainty);

  moduleParamList->addParameter(prefixed(prefix, "overwriteExistingEstimation"), m_param_overwriteExistingEstimation,
                                "Whether to replace an existing time estimation or not.",
                                m_param_overwriteExistingEstimation);

  moduleParamList->addParameter(prefixed(prefix, "maximumTracksUsed"), m_param_maximumTracksUsed,
                                "the maximum amount of tracks used for the extraction.",
                                m_param_maximumTracksUsed);
  moduleParamList->addParameter(prefixed(prefix, "minimumTrackPt"), m_param_minimumTrackPt,
                                "Track Pt a track must at least have to be used for time fitting",
                                m_param_minimumTrackPt);
}

bool FullGridTrackTimeExtraction::wasSuccessful() const
{
  return m_wasSuccesful;
}

void FullGridTrackTimeExtraction::apply(std::vector< RecoTrack*>& allRecoTracks)
{
  m_wasSuccesful = false;

  if (allRecoTracks.size() == 0) {
    B2DEBUG(50, "No RecoTracks to perform t0 grid extraction, skipping for this event");
    return;
  }

  auto selectedRecoTracks = TimeExtractionUtils::selectTracksForTimeExtraction(allRecoTracks, m_param_minimalNumberCDCHits,
                            m_param_maximumTracksUsed,
                            m_param_minimumTrackPt);

  // check if there are any reco tracks at all available
  if (selectedRecoTracks.size() == 0) {
    B2DEBUG(50, "No tracks for time extraction satisfy the requirements, skipping this event ");
    return;
  }

  B2DEBUG(50, "Found " << selectedRecoTracks.size() << " which can be used for time extraction.");

  if (not m_eventT0.isValid()) {
    m_eventT0.create();
  } else if (not m_param_overwriteExistingEstimation) {
    B2INFO("T0 estimation already present and overwriteExistingEstimation set to false. Skipping.");
    return;
  }

  // store the previous best t0 extraction for CDC, if present
  std::tuple <bool, double, double> initialT0 = {false, 0.0f, 0.0f};
  if (m_eventT0->hasEventT0()) {
    initialT0 = {true, m_eventT0->getEventT0(), m_eventT0->getEventT0Uncertainty() };
  }

  // Try out phase: test 3 data points between t0 min and t0 max and let them extrapolate a bit.
  std::vector<T0Try> tries;
  std::vector<T0Try> convergedTries;

  const double deltaT0 = (1.0 / double(m_param_numberOfGrids)) * (m_param_maximalT0Shift - m_param_minimalT0Shift);

  for (double i = 1; i < m_param_numberOfGrids; i++) {
    extractTrackTimeFrom(selectedRecoTracks, m_param_minimalT0Shift + i * deltaT0, 2, tries, convergedTries,
                         m_param_minimalT0Shift, m_param_maximalT0Shift);
  }

  if (not convergedTries.empty()) {
    // If we have found some "converging" extracted t0s, use the one with the lowest chi2.
    const auto& minimalChi2 = std::min_element(convergedTries.begin(), convergedTries.end());

    const double extractedTime = minimalChi2->m_extractedT0;
    // The uncertainty was calculated using a test MC sample
    B2DEBUG(50, "Converged T0 fit with the lowest chi2 is " << extractedTime << " ns");
    m_eventT0->addTemporaryEventT0(extractedTime, m_param_t0Uncertainty, Const::EDetector::CDC);
    m_wasSuccesful = true;
  } else {
    // If not, start with the lowest extracted chi2 and do another two iteration steps. If it converges then,
    // use this. Else, use the next best guess.
    std::sort(tries.begin(), tries.end());

    for (const auto& tryOut : tries) {
      extractTrackTimeFrom(selectedRecoTracks, tryOut.m_extractedT0, 2, tries, convergedTries, m_param_minimalT0Shift,
                           m_param_maximalT0Shift);
      if (not convergedTries.empty()) {
        const double extractedTime = convergedTries.back().m_extractedT0;

        B2DEBUG(50, "T0 fit after 2 additional tries with the lowest chi2 is " << extractedTime << " ns");
        m_eventT0->addTemporaryEventT0(extractedTime, m_param_t0Uncertainty, Const::EDetector::CDC);
        m_wasSuccesful = true;
        break;
      } else {
        B2DEBUG(50, "Still no converged fit after additional tries, t0 extraction failed.");
      }
    }
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

