/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                           *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/fitter/timeEstimator/BaseTrackTimeEstimatorModule.h>

#include <tracking/trackFitting/fitter/base/TrackFitter.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <TVector3.h>

using namespace std;
using namespace Belle2;

namespace {
  /// Anonymous helper function for doing some maths.
  double calculateVelocity(const TVector3& momentum, const Const::ChargedStable& particleHypothesis)
  {
    // Particle velocity in cm / ns using the typical relation between E and p.
    const double m = particleHypothesis.getMass();
    const double p = momentum.Mag();
    const double E = hypot(m, p);
    const double beta = p / E;
    const double v = beta * Const::speedOfLight;

    return v;
  }
}

BaseTrackTimeEstimatorModule::BaseTrackTimeEstimatorModule() :
  Module()
{
  setDescription("Module estimating the track time of RecoTracks - before or after the fit. "
                 "Loops over all RecoTracks and set their time seed correctly. In case of using the fitted information,"
                 "it also sets the track seeds of the position and momentum into the first measurement (where the time seed"
                 "is calculated). It also deletes all fitted information. Do not forget to refit "
                 "the tracks afterwards.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("recoTracksStoreArrayName", m_param_recoTracksStoreArrayName, "StoreArray name of the input and output reco tracks.",
           m_param_recoTracksStoreArrayName);

  addParam("useFittedInformation", m_param_useFittedInformation, "Whether to use the information in the measurements (after fit)"
           " or the tracking seeds for doing the extrapolation. Of course, the track fit has to be performed to use the fitted information.",
           m_param_useFittedInformation);

  addParam("pdgCodeToUseForEstimation", m_param_pdgCodeToUseForEstimation,
           "Which PDG code to use for creating the time estimate. How this information is used"
           "depends on the implementation details of the child modules. Please only use the positive pdg code.",
           m_param_pdgCodeToUseForEstimation);

  addParam("timeOffset", m_param_timeOffset, "If you want to subtract or add a certain time, you can use this variable.",
           m_param_timeOffset);

  addParam("readoutPosition", m_param_readoutPosition,
           "In cases where the readout of the trigger is not located at the trigger directly and the signal has to"
           "propagate a non-vanashing distance, you can set the readout position here. Please note that you have to"
           "enable this feature by using the useReadoutPosition flag. You can control the propagation speed with the"
           "flag readoutPositionPropagationSpeed.", m_param_readoutPosition);
  addParam("useReadoutPosition", m_param_useReadoutPosition, "Enable the usage of the readout position."
           "When this feature is enabled, the length from the incident of the particle in the trigger to the position"
           "set by the readoutPosition flag is calculated and using the readoutPositionPropagationSpeed, a time is"
           "calculated which is used in the time estimation as an offset."
           "In the moment, this feature is only possible when using the fitted information." , m_param_useReadoutPosition);
  addParam("readoutPositionPropagationSpeed", m_param_readoutPositionPropagationSpeed,
           "Speed of the propagation from the hit on the trigger to the readoutPosition. Only is used when the"
           "flag useReadoutPosition is enabled.", m_param_readoutPositionPropagationSpeed);
}

void BaseTrackTimeEstimatorModule::initialize()
{
  // Read and write out RecoTracks
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  if (m_param_useReadoutPosition and not m_param_useFittedInformation) {
    B2FATAL("The combination of using the seed information and the readout position is not implemented in the moment.");
  }
}

void BaseTrackTimeEstimatorModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);

  const Const::ChargedStable particleHypothesis(m_param_pdgCodeToUseForEstimation);

  // Estimate the track time for each reco track depending on the settings of the module.
  for (auto& recoTrack : recoTracks) {
    double timeSeed;
    if (m_param_useFittedInformation) {
      try {
        timeSeed = estimateTimeSeedUsingFittedInformation(recoTrack, particleHypothesis);
      } catch (genfit::Exception& e) {
        B2WARNING("Time extraction from fitted state failed because of " << e.what());
        timeSeed = -9999;
      }
    } else {
      timeSeed = estimateTimeSeedUsingSeedInformation(recoTrack, particleHypothesis);
    }

    if (!(timeSeed > -1000)) {
      // Guard against NaN or just something silly.
      B2WARNING("Fixing calculated seed Time " << timeSeed << " to zero.");
      timeSeed = 0;
    } else {
      // Add the constant time offset only in non-silly cases.
      timeSeed += m_param_timeOffset;
    }

    B2DEBUG(100, "Setting seed to " <<  timeSeed);
    recoTrack.setTimeSeed(timeSeed);

    // Delete all fitted information for all representations
    recoTrack.deleteFittedInformation();
  }
}

double BaseTrackTimeEstimatorModule::estimateTimeSeedUsingFittedInformation(RecoTrack& recoTrack,
    const Const::ChargedStable& particleHypothesis) const
{
  const int currentPdgCode = TrackFitter::createCorrectPDGCodeForChargedStable(particleHypothesis, recoTrack);
  const genfit::AbsTrackRep* trackRepresentation = TrackFitter::getTrackRepresentationForPDG(currentPdgCode, recoTrack);

  if (not trackRepresentation or not recoTrack.wasFitSuccessful(trackRepresentation)) {
    B2WARNING("Could not estimate a correct time, as the last fit failed.");
    return 0;
  } else {
    // If the flight length is clear, just use the s = v * t relation.
    genfit::MeasuredStateOnPlane measuredState = recoTrack.getMeasuredStateOnPlaneFromFirstHit(trackRepresentation);

    // Fix the position and momentum seed to the same place as where we calculation the time seed: the first measured state on plane
    recoTrack.setPositionAndMomentum(measuredState.getPos(), measuredState.getMom());

    const double flightLength = estimateFlightLengthUsingFittedInformation(measuredState);

    // Be aware that we use the measured state on plane after the extrapolation to compile the momentum.
    const TVector3& momentum = measuredState.getMom();
    const double v = calculateVelocity(momentum, particleHypothesis);

    const double flightTime = flightLength / v;

    // When the readout position should be used, calculate the propagation time of the signal from the hit to the
    // readout position.
    if (m_param_useReadoutPosition) {
      const TVector3& position = measuredState.getPos();
      B2ASSERT("Readout Position must have 3 components.", m_param_readoutPosition.size() == 3);
      const TVector3 readoutPositionAsTVector3(m_param_readoutPosition[0], m_param_readoutPosition[1], m_param_readoutPosition[2]);
      const double propagationLength = (position - readoutPositionAsTVector3).Mag();
      const double propagationTime = propagationLength / m_param_readoutPositionPropagationSpeed;

      return flightTime - propagationTime;
    } else {
      return flightTime;
    }
  }
}

double BaseTrackTimeEstimatorModule::estimateTimeSeedUsingSeedInformation(RecoTrack& recoTrack,
    const Const::ChargedStable& particleHypothesis) const
{
  // If the flight length is clear, just use the s = v * t relation.
  const double s = estimateFlightLengthUsingSeedInformation(recoTrack);

  const TVector3& momentum = recoTrack.getMomentumSeed();
  const double v = calculateVelocity(momentum, particleHypothesis);

  return s / v;
}
