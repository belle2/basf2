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

#include <genfit/AbsTrackRep.h>
#include <genfit/FitStatus.h>
#include <genfit/AbsFitter.h>
#include <genfit/DAF.h>
#include <genfit/RKTrackRep.h>
#include <genfit/KalmanFitterInfo.h>

using namespace Belle2;

constexpr double TrackFitter::s_defaultDeltaPValue;
constexpr double TrackFitter::s_defaultProbCut;
constexpr unsigned int TrackFitter::s_defaultMaxFailedHits;

int TrackFitter::createCorrectPDGCodeForChargedStable(const Const::ChargedStable& particleType, const RecoTrack& recoTrack)
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

  return currentPdgCode;
}

bool TrackFitter::fit(RecoTrack& recoTrack) const
{
  if (not recoTrack.getRepresentations().empty() and recoTrack.getCardinalRepresentation()) {
    return fit(recoTrack, recoTrack.getCardinalRepresentation());
  } else {
    return fit(recoTrack, Const::pion);
  }
}

bool TrackFitter::fit(RecoTrack& recoTrack, const Const::ChargedStable& particleType) const
{
  const int currentPdgCode = TrackFitter::createCorrectPDGCodeForChargedStable(particleType, recoTrack);
  genfit::AbsTrackRep* trackRepresentation = RecoTrackGenfitAccess::createOrReturnRKTrackRep(recoTrack,
                                             currentPdgCode);

  return fit(recoTrack, trackRepresentation);
}

bool TrackFitter::fitWithoutCheck(RecoTrack& recoTrack, const genfit::AbsTrackRep& trackRepresentation) const
{
  // Fit the track
  try {
    // Delete the old information to start from scratch
    recoTrack.deleteFittedInformationForRepresentation(&trackRepresentation);
    m_fitter->processTrackWithRep(&RecoTrackGenfitAccess::getGenfitTrack(recoTrack), &trackRepresentation);
  } catch (genfit::Exception& e) {
    B2WARNING(e.getExcString());
  }

  recoTrack.setDirtyFlag(false);

  // Do the hits synchronisation
  const std::vector<RecoHitInformation*>& relatedRecoHitInformation = recoTrack.getRecoHitInformations();

  for (RecoHitInformation* recoHitInformation : relatedRecoHitInformation) {
    const genfit::TrackPoint* trackPoint = recoTrack.getCreatedTrackPoint(recoHitInformation);
    if (trackPoint) {
      genfit::KalmanFitterInfo* kalmanFitterInfo = trackPoint->getKalmanFitterInfo(&trackRepresentation);
      if (not kalmanFitterInfo) {
        recoHitInformation->setFlag(RecoHitInformation::RecoHitFlag::c_dismissedByFit);
      } else {
        std::vector<double> weights = kalmanFitterInfo->getWeights();
        for (const double weight : weights) {
          if (weight == 0) {
            recoHitInformation->setFlag(RecoHitInformation::RecoHitFlag::c_dismissedByFit);
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

  if (RecoTrackGenfitAccess::getGenfitTrack(recoTrack).getNumPoints() == 0) {
    B2WARNING("No track points (measurements) were added to this reco track. Have you used an invalid measurement adder?");
    return false;
  }

  const std::vector<genfit::AbsTrackRep*>& trackRepresentations = recoTrack.getRepresentations();
  if (std::find(trackRepresentations.begin(), trackRepresentations.end(), trackRepresentation) == trackRepresentations.end()) {
    B2FATAL("The TrackRepresentation provided is not part of the Reco Track.");
  }

  if (not recoTrack.getDirtyFlag() and not m_skipDirtyCheck and not measurementAdderNeedsTrackRefit
      and recoTrack.hasTrackFitStatus(trackRepresentation) and recoTrack.getTrackFitStatus(trackRepresentation)->isFitted()) {
    B2DEBUG(100, "Hit content did not change, track representation is already present and you used only default parameters." <<
            "I will not fit the track again. If you still want to do so, set the dirty flag of the track.");
    return recoTrack.wasFitSuccessful(trackRepresentation);
  }

  const auto previousSetting = gErrorIgnoreLevel; // Save current log level
  gErrorIgnoreLevel = m_gErrorIgnoreLevel; // Set the log level defined in the TrackFitter
  auto fitWithoutCheckResult = fitWithoutCheck(recoTrack, *trackRepresentation);
  gErrorIgnoreLevel = previousSetting; // Restore previous setting
  return fitWithoutCheckResult;
}

void TrackFitter::resetFitterToDefaultSettings()
{
  genfit::DAF* dafFitter = new genfit::DAF(true, s_defaultDeltaPValue);
  dafFitter->setProbCut(s_defaultProbCut);
  dafFitter->setMaxFailedHits(s_defaultMaxFailedHits);

  m_fitter.reset(dafFitter);

  m_skipDirtyCheck = false;
}

void TrackFitter::resetFitter(const std::shared_ptr<genfit::AbsFitter>& fitter)
{
  m_fitter = fitter;
  m_skipDirtyCheck = true;
}