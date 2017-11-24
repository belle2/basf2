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

void TrackTimeExtraction::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{

  moduleParamList->addParameter(prefixed(prefix, "recoTracksStoreArrayName"), m_param_recoTracksStoreArrayName,
                                "StoreArray containing the RecoTracks to process",
                                m_param_recoTracksStoreArrayName);

  moduleParamList->addParameter(prefixed(prefix, "maximalIterations"), m_param_maximalIterations,
                                "Maximal number of iterations to perform.",
                                m_param_maximalIterations);
  moduleParamList->addParameter(prefixed(prefix, "minimalIterations"), m_param_minimalIterations,
                                "Minimal number of iterations to perform.",
                                m_param_minimalIterations);

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
}


void TrackTimeExtraction::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  m_eventT0.registerInDataStore();
}

void TrackTimeExtraction::apply(std::vector<RecoTrack*>& recoTracks)
{
  if (not m_eventT0.isValid()) {
    m_eventT0.create();
  } else if (not m_param_overwriteExistingEstimation) {
    B2INFO("T0 estimation already present and overwriteExistingEstimation set to false. Skipping.");
    return;
  }

  const double extractedTime = extractTrackTimeLoop(recoTracks);

  // The uncertainty was calculated using a test MC sample
  m_eventT0->addEventT0(extractedTime, m_param_t0Uncertainty, Const::EDetector::CDC);
}

double TrackTimeExtraction::extractTrackTimeLoop(std::vector<RecoTrack*>& recoTracks) const
{
  TrackFitter trackFitter;

  double sumExtractedTime = 0;
  double lastExtractedTime = std::nan("");

  unsigned int loopCounter = 0;

  for (; loopCounter < m_param_maximalIterations; loopCounter++) {
    for (RecoTrack* recoTrack : recoTracks) {
      trackFitter.fit(*recoTrack);
    }

    double randomizeValue = lastExtractedTime;
    if (std::isnan(randomizeValue)) {
      randomizeValue = m_param_randomizeLimits * (1 - 0.5 * loopCounter / m_param_maximalIterations);
    }

    const double extractedTime = extractTrackTime(recoTracks, randomizeValue);
    B2INFO(extractedTime);
    if (std::isnan(extractedTime)) {
      B2ERROR("Extracted Time is NaN! Aborting.");
      break;
    } else {
      sumExtractedTime += extractedTime;
      lastExtractedTime = extractedTime;
      if (fabs(extractedTime) < m_param_minimalTimeDeviation and loopCounter >= m_param_minimalIterations) {
        B2RESULT("Final delta T0 " << extractedTime << " with " << sumExtractedTime
                 << ". Needed " << loopCounter << " iterations.");
        break;
      }
    }
  }

  if (loopCounter == m_param_maximalIterations) {
    B2WARNING("Could not determine the track time in the maximum number of iterations to the needed precision.");
  }

  return sumExtractedTime;
}



double TrackTimeExtraction::extractTrackTime(std::vector<RecoTrack*>& recoTracks, const double& randomizeLimits) const
{
  double sumFirstDerivatives = 0;
  double sumSecondDerivatives = 0;
  for (RecoTrack* recoTrack : recoTracks) {
    const auto& chi2Derivatives = TimeExtractionUtils::getChi2Derivatives(*recoTrack);
    const double& dchi2da = chi2Derivatives.first;
    const double& d2chi2da2 = chi2Derivatives.second;

    if (not std::isnan(dchi2da) and not std::isnan(d2chi2da2)) {
      if (d2chi2da2 > 20) {
        B2DEBUG(200, "Track with bad second derivative");
        continue;
      }
      sumFirstDerivatives += d2chi2da2;
      sumSecondDerivatives += dchi2da;
    }
  }

  double extractedT0 = 0;

  if (sumSecondDerivatives != 0 or sumFirstDerivatives != 0) {
    extractedT0 = sumSecondDerivatives / sumFirstDerivatives;
  } else {
    if (m_param_randomizeOnError) {
      extractedT0 = gRandom->Uniform(-randomizeLimits, randomizeLimits);
      B2WARNING("Strange results, setting randomly to " << extractedT0);
    }
  }

  if (fabs(extractedT0) > m_param_maximalExtractedT0) {
    extractedT0 = std::copysign(m_param_maximalExtractedT0, extractedT0);
  }

  if (extractedT0 != 0) {
    for (RecoTrack* recoTrack : recoTracks) {
      recoTrack->setTimeSeed(recoTrack->getTimeSeed() + extractedT0);
      recoTrack->deleteFittedInformation();
    }
  }

  return extractedT0;
}
