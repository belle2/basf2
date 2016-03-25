/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <framework/dataobjects/Helix.h>

#include <genfit/AbsTrackRep.h>
#include <genfit/FitStatus.h>
#include <genfit/AbsFitter.h>
#include <genfit/DAF.h>
#include <genfit/RKTrackRep.h>
#include <genfit/KalmanFitterInfo.h>

using namespace Belle2;

bool TrackFitter::fit(RecoTrack& recoTrack, const Const::ChargedStable& particleType) const
{
  int currentPdgCode = particleType.getPDGCode();

  const auto& pdgParticleCharge = particleType.getParticlePDG()->Charge();
  const auto& recoTrackCharge = recoTrack.getChargeSeed();

  // Copy from GenfitterModule
  B2ASSERT("Charge of candidate and PDG particle don't match.  (Code assumes |q| = 1).",
           fabs(pdgParticleCharge) == fabs(recoTrackCharge * 3.0));

  /*
   * Because the charged stable particles do describe a positive as well as a negative particle,
   * we have to correct the charge if needed.
   */
  if (std::signbit(pdgParticleCharge) != std::signbit(recoTrackCharge))
    currentPdgCode *= -1;

  const std::vector<genfit::AbsTrackRep*>& trackRepresentations = recoTrack.m_genfitTrack.getTrackReps();
  genfit::AbsTrackRep* alreadyPresentTrackRepresentation = nullptr;
  for (genfit::AbsTrackRep* trackRepresentation : trackRepresentations) {
    // Check if the track representation is a RKTrackRep.
    const genfit::RKTrackRep* rkTrackRepresenation = dynamic_cast<const genfit::RKTrackRep*>(trackRepresentation);
    if (rkTrackRepresenation != nullptr) {
      if (rkTrackRepresenation->getPDG() == currentPdgCode) {
        alreadyPresentTrackRepresentation = trackRepresentation;
        break;
      }
    }
  }

  if (alreadyPresentTrackRepresentation) {
    B2DEBUG(100, "Reusing the already present track representation with the same PDG code.");
    return fit(recoTrack, alreadyPresentTrackRepresentation);
  } else {
    genfit::AbsTrackRep* newTrackRep = new genfit::RKTrackRep(currentPdgCode);
    return fit(recoTrack, newTrackRep);
  }
}

bool TrackFitter::fitWithoutCheck(RecoTrack& recoTrack, const genfit::AbsTrackRep& trackRepresentation) const
{
  recoTrack.setTimeSeed(calculateTimeSeed(recoTrack, trackRepresentation));
  // Fit the track
  m_fitter->processTrack(&recoTrack.m_genfitTrack, false);
  recoTrack.setDirtyFlag(false);

  // Do the hits synchronisation
  auto relatedRecoHitInformation =
    recoTrack.getRelationsTo<RecoHitInformation>(recoTrack.getStoreArrayNameOfRecoHitInformation());

  for (RecoHitInformation& recoHitInformation : relatedRecoHitInformation) {
    const genfit::TrackPoint* trackPoint = recoHitInformation.getCreatedTrackPoint();
    if (trackPoint) {
      genfit::KalmanFitterInfo* kalmanFitterInfo = trackPoint->getKalmanFitterInfo(&trackRepresentation);
      if (not kalmanFitterInfo) {
        recoHitInformation.setFlag(RecoHitInformation::RecoHitFlag::c_dismissedByFit);
      } else {
        std::vector<double> weights = kalmanFitterInfo->getWeights();
        for (const double weight : weights) {
          if (weight == 0) {
            recoHitInformation.setFlag(RecoHitInformation::RecoHitFlag::c_dismissedByFit);
          }
        }
      }
    }
  }

  return recoTrack.wasFitSuccessful(&trackRepresentation);
}

bool TrackFitter::fit(RecoTrack& recoTrack, genfit::AbsTrackRep* trackRepresentation) const
{
  B2ASSERT("No fitter was loaded! Have you reset the fitter to an invalid one?", m_fitter);

  const bool measurementAdderNeedsTrackRefit = m_measurementAdder.addMeasurements(recoTrack);

  if (recoTrack.m_genfitTrack.getNumPoints() == 0) {
    B2WARNING("No track points (measurements) were added to this reco track. Have you used an invalid measurement adder?");
    return false;
  }

  const std::vector<genfit::AbsTrackRep*>& trackRepresentations = recoTrack.m_genfitTrack.getTrackReps();
  if (std::find(trackRepresentations.begin(), trackRepresentations.end(), trackRepresentation) == trackRepresentations.end()) {
    recoTrack.m_genfitTrack.addTrackRep(trackRepresentation);
  } else {
    if (not recoTrack.getDirtyFlag() and not m_skipDirtyCheck and not measurementAdderNeedsTrackRefit) {
      B2WARNING("Hit content did not change, track representation is already present and you used only default parameters." <<
                "I will not fit the track again. If you still want to do so, set the dirty flag of the track.");
      return recoTrack.wasFitSuccessful(trackRepresentation);
    }
  }

  return fitWithoutCheck(recoTrack, *trackRepresentation);
}

double TrackFitter::calculateTimeSeed(const RecoTrack& recoTrack,
                                      const genfit::AbsTrackRep& trackRepresentation) const
{
  const int pdgCodeForFit = trackRepresentation.getPDG();
  TParticlePDG* particleWithPDGCode = TDatabasePDG::Instance()->GetParticle(pdgCodeForFit);
  const TVector3& momentum = recoTrack.getMomentumSeed();

  // Particle velocity in cm / ns.
  const double m = particleWithPDGCode->Mass();
  const double p = momentum.Mag();
  const double E = hypot(m, p);
  const double beta = p / E;
  const double v = beta * Const::speedOfLight;

  // Arc length from IP to posSeed in cm.
  // Calculate the arc-length.  Helix doesn't provide a way of
  // obtaining this directly from the difference in z, as it
  // only provide arc-lengths in the transverse plane, so we do
  // it like this.
  const TVector3& perigeePosition = recoTrack.getPositionSeed();
  const Belle2::Helix h(perigeePosition, momentum, particleWithPDGCode->Charge() / 3, 1.5);
  const double s2D = h.getArcLength2DAtCylindricalR(perigeePosition.Perp());
  const double s = s2D * hypot(1, h.getTanLambda());

  // Time (ns) from trigger (= 0 ns) to posSeed assuming constant velocity.
  double timeSeed = s / v;

  if (!(timeSeed > -1000)) {
    // Guard against NaN or just something silly.
    B2WARNING("Fixing calculated seed Time " << timeSeed << " to zero.");
    timeSeed = 0;
  }

  return timeSeed;
}

void TrackFitter::resetFitterToDefaultSettings()
{
  genfit::DAF* dafFitter = new genfit::DAF(true);
  dafFitter->setProbCut(0.001);
  dafFitter->setMaxFailedHits(5);

  m_fitter.reset(dafFitter);

  m_skipDirtyCheck = false;
}
