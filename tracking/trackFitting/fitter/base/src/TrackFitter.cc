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

bool TrackFitter::fit(RecoTrack& recoTrack, const Const::ChargedStable& particleType) const
{
  const int currentPdgCode = TrackFitter::createCorrectPDGCodeForChargedStable(particleType, recoTrack);
  genfit::AbsTrackRep* alreadyPresentTrackRepresentation = TrackFitter::getTrackRepresentationForPDG(currentPdgCode, recoTrack);

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
  // Fit the track
  try {
    m_fitter->processTrack(&RecoTrackGenfitAccess::getGenfitTrack(recoTrack), false);
  } catch (genfit::Exception& e) {
    B2WARNING(e.getExcString());
  }

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

  if (RecoTrackGenfitAccess::getGenfitTrack(recoTrack).getNumPoints() == 0) {
    B2WARNING("No track points (measurements) were added to this reco track. Have you used an invalid measurement adder?");
    return false;
  }

  const std::vector<genfit::AbsTrackRep*>& trackRepresentations = recoTrack.getRepresentations();
  if (std::find(trackRepresentations.begin(), trackRepresentations.end(), trackRepresentation) == trackRepresentations.end()) {
    RecoTrackGenfitAccess::getGenfitTrack(recoTrack).addTrackRep(trackRepresentation);
  } else {
    if (not recoTrack.getDirtyFlag() and not m_skipDirtyCheck and not measurementAdderNeedsTrackRefit) {
      B2DEBUG(100, "Hit content did not change, track representation is already present and you used only default parameters." <<
              "I will not fit the track again. If you still want to do so, set the dirty flag of the track.");
      return recoTrack.wasFitSuccessful(trackRepresentation);
    }
  }

  return fitWithoutCheck(recoTrack, *trackRepresentation);
}

void TrackFitter::resetFitterToDefaultSettings()
{
  genfit::DAF* dafFitter = new genfit::DAF(true, m_dafDeltaPval);
  dafFitter->setProbCut(0.001);
  dafFitter->setMaxFailedHits(5);

  m_fitter.reset(dafFitter);

  m_skipDirtyCheck = false;
}
