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
}

void BaseTrackTimeEstimatorModule::initialize()
{
  // Read and write out RecoTracks
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();
}

void BaseTrackTimeEstimatorModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);

  const Const::ChargedStable particleHypothesis(m_param_pdgCodeToUseForEstimation);

  // Estimate the track time for each reco track depending on the settings of the module.
  for (auto& recoTrack : recoTracks) {
    double timeSeed;
    if (m_param_useFittedInformation) {
      timeSeed = estimateTimeSeedUsingFittedInformation(recoTrack, particleHypothesis);
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
    for (const genfit::AbsTrackRep* trackRep : recoTrack.getRepresentations()) {
      for (genfit::TrackPoint* hit : recoTrack.getHitPointsWithMeasurement()) {
        hit->deleteFitterInfo(trackRep);
      }
    }
  }
}

double BaseTrackTimeEstimatorModule::estimateTimeSeedUsingFittedInformation(RecoTrack& recoTrack,
    const Const::ChargedStable& particleHypothesis) const
{
  if (not recoTrack.wasFitSuccessful()) {
    B2WARNING("Could not estimate a correct time, as the last fit failed.");
    return 0;
  } else {
    const int currentPdgCode = TrackFitter::createCorrectPDGCodeForChargedStable(particleHypothesis, recoTrack);
    const genfit::AbsTrackRep* trackRepresentation = TrackFitter::getTrackRepresentationForPDG(currentPdgCode, recoTrack);

    if (not trackRepresentation) {
      B2WARNING("Reco Track was not fitted with this hypothesis. Will use the cardinal one.");
      trackRepresentation = recoTrack.getCardinalRepresentation();
    }

    // If the flight length is clear, just use the s = v * t relation.
    genfit::MeasuredStateOnPlane measuredState = recoTrack.getMeasuredStateOnPlaneFromFirstHit(trackRepresentation);

    // Fix the position and momentum seed to the same place as where we calculation the time seed: the first measured state on plane
    recoTrack.setPositionAndMomentum(measuredState.getPos(), measuredState.getMom());

    const double s = estimateFlightLengthUsingFittedInformation(measuredState);

    // Be aware that we use the measured state on plane after the extrapolation to compile the momentum.
    const TVector3& momentum = measuredState.getMom();
    const double v = calculateVelocity(momentum, particleHypothesis);

    return s / v;
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
