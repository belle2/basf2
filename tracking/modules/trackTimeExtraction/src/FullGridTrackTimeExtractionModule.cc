/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter, Thomas Hauth, Nils Braun                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackTimeExtraction/FullGridTrackTimeExtractionModule.h>
#include <tracking/timeExtraction/TimeExtractionUtils.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

using namespace Belle2;

REG_MODULE(FullGridTrackTimeExtraction);

namespace {
  /// Helper Structure holding one extracted time together with their chi^2.
  struct T0Try {
    double m_extractedT0;
    double m_chi2;

    T0Try(const double& extractedT0, const double& chi2) :
      m_extractedT0(extractedT0), m_chi2(chi2) {}
  };

  /// Make a list of extracted times sortable by their chi^2.
  bool operator<(const T0Try& lhs, const T0Try& rhs)
  {
    return lhs.m_chi2 < rhs.m_chi2;
  }

  /**
   * Main function used in the module: Repeat the time extraction of the reco track starting from the given start
   * value until:
   * (a) the maximum number of steps is reached
   * (b) the extracted time is not in the range [t0 min, t0 max]
   * (c) the fit needed in the extraction fails
   * (d) the result is non-sense (e.g. NaN)
   *
   * Each extracted time is either added to the tries or the converged tries list, depending on if the chi^2 is small
   * and d^2 chi^2 / (d alpha)^2 is large enough.
   *
   * In the end, the track is reset to have the initial time seed.
   */
  void extractTrackTimeFrom(RecoTrack& recoTrack, const double& startValue, const unsigned int steps,
                            std::vector<T0Try>& tries, std::vector<T0Try>& convergedTries,
                            const double& minimalT0, const double& maximalT0)
  {
    TrackFitter trackFitter;

    const double initialValue = recoTrack.getTimeSeed();

    recoTrack.setTimeSeed(initialValue + startValue);
    recoTrack.deleteFittedInformation();

    trackFitter.fit(recoTrack);

    if (recoTrack.wasFitSuccessful()) {
      const double chi2 = TimeExtractionUtils::extractReducedChi2(recoTrack);

      const double recoTrack_time = recoTrack.getTimeSeed();
      tries.emplace_back(recoTrack_time, chi2);

      for (unsigned int i = 0; i < steps; i++) {
        const double extracted_time = recoTrack.getTimeSeed() + TimeExtractionUtils::extractTime(recoTrack);
        if (extracted_time > maximalT0 or extracted_time < minimalT0 or std::isnan(extracted_time)) {
          break;
        }

        const auto& derivatives_pair = TimeExtractionUtils::getChi2Derivatives(recoTrack);

        recoTrack.setTimeSeed(extracted_time);
        recoTrack.deleteFittedInformation();

        trackFitter.fit(recoTrack);

        if (not recoTrack.wasFitSuccessful()) {
          break;
        }

        const double chi2 = TimeExtractionUtils::extractReducedChi2(recoTrack);

        if (chi2 > 10) {
          break;
        }

        const bool finished = derivatives_pair.second > 2.7122 and chi2 < 1.739;

        if (finished) {
          convergedTries.emplace_back(extracted_time, chi2);
          break;
        } else {
          tries.emplace_back(extracted_time, chi2);
        }
      }
    }

    recoTrack.setTimeSeed(initialValue);
    recoTrack.deleteFittedInformation();
  }

}


FullGridTrackTimeExtractionModule::FullGridTrackTimeExtractionModule() : Module()
{
  setDescription("Build the full covariance matrix for RecoTracks and extract the event time using the CDC drift time information.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("recoTracksStoreArrayName", m_param_recoTracksStoreArrayName, "StoreArray containing the RecoTracks to process",
           m_param_recoTracksStoreArrayName);

  addParam("minimalT0Shift", m_param_minimalT0Shift, "Minimal shift of the event time which is allowed.",
           m_param_minimalT0Shift);
  addParam("maximalT0Shift", m_param_maximalT0Shift, "Maximal shift of the event time which is allowed.",
           m_param_maximalT0Shift);

  addParam("t0Uncertainty", m_param_t0Uncertainty, "Use this as sigma t0.",
           m_param_t0Uncertainty);

  addParam("overwriteExistingEstimation", m_param_overwriteExistingEstimation,
           "Whether to replace an existing time estimation or not.",
           m_param_overwriteExistingEstimation);

}

void FullGridTrackTimeExtractionModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  m_eventT0.registerInDataStore();
}

void FullGridTrackTimeExtractionModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);

  if (recoTracks.getEntries() > 1) {
    B2FATAL("More than one RecoTrack is not allowed in the StoreArray. " <<
            "There are " << recoTracks.getEntries() << " RecoTracks in the array.");
    return;
  } else if (recoTracks.getEntries() == 0) {
    return;
  }

  if (not m_eventT0.isValid()) {
    m_eventT0.create();
  } else if (not m_param_overwriteExistingEstimation) {
    B2INFO("T0 estimation already present and overwriteExistingEstimation set to false. Skipping.");
    return;
  }

  // Try out phase: test 3 data points between t0 min and t0 max and let them extrapolate a bit.
  std::vector<T0Try> tries;
  std::vector<T0Try> convergedTries;

  const double deltaT0 = 0.2 * (m_param_maximalT0Shift - m_param_minimalT0Shift);

  RecoTrack& recoTrack = *(recoTracks[0]);
  extractTrackTimeFrom(recoTrack, m_param_minimalT0Shift + 1 * deltaT0, 2, tries, convergedTries, m_param_minimalT0Shift,
                       m_param_maximalT0Shift);
  extractTrackTimeFrom(recoTrack, m_param_minimalT0Shift + 2 * deltaT0, 2, tries, convergedTries, m_param_minimalT0Shift,
                       m_param_maximalT0Shift);
  extractTrackTimeFrom(recoTrack, m_param_minimalT0Shift + 3 * deltaT0, 2, tries, convergedTries, m_param_minimalT0Shift,
                       m_param_maximalT0Shift);
  extractTrackTimeFrom(recoTrack, m_param_minimalT0Shift + 4 * deltaT0, 2, tries, convergedTries, m_param_minimalT0Shift,
                       m_param_maximalT0Shift);

  if (not convergedTries.empty()) {
    // If we have found some "converging" extracted t0s, use the one with the lowest chi2.
    const auto& minimalChi2 = std::min_element(convergedTries.begin(), convergedTries.end());

    const double extractedTime = minimalChi2->m_extractedT0;
    // The uncertainty was calculated using a test MC sample
    m_eventT0->addEventT0(extractedTime, m_param_t0Uncertainty, Const::EDetector::CDC);
  } else {
    // If not, start with the lowest extracted chi2 and do another two iteration steps. If it converges then,
    // use this. Else, use the next best guess.
    std::sort(tries.begin(), tries.end());

    for (const auto& tryOut : tries) {
      extractTrackTimeFrom(recoTrack, tryOut.m_extractedT0, 2, tries, convergedTries, m_param_minimalT0Shift, m_param_maximalT0Shift);
      if (not convergedTries.empty()) {
        const double extractedTime = convergedTries.back().m_extractedT0;
        // The uncertainty was calculated using a test MC sample
        m_eventT0->addEventT0(extractedTime, m_param_t0Uncertainty, Const::EDetector::CDC);
        break;
      }
    }
  }
}

