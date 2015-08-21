/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/fitter/TrackBuilderFromRecoTracksModule.h>

#include <tracking/modules/genfitter/TrackBuilderModule.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/FieldManager.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>

using namespace std;
using namespace Belle2;

REG_MODULE(TrackBuilderFromRecoTracks)

TrackBuilderFromRecoTracksModule::TrackBuilderFromRecoTracksModule() :
  Module()
{
  setDescription("Create Belle::tracks from the given reco Tracks.");

  addParam("RecoTracksStoreArrayName", m_param_recoTracksStoreArrayName, "StoreArray name of the input and output reco tracks.",
           std::string("RecoTracks"));

  addParam("TrackCandidatesStoreArrayName", m_param_trackCandidatesStoreArrayName,
           "StoreArray name of the input track candidates related to the reco tracks.",
           std::string("TrackCands"));

  addParam("TracksStoreArrayName", m_param_tracksStoreArrayName, "StoreArray name of the output genfit::tracks.",
           std::string("Tracks"));

}

void TrackBuilderFromRecoTracksModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  StoreArray<genfit::TrackCand> trackCands(m_param_trackCandidatesStoreArrayName);
  trackCands.isRequired();

  StoreArray<Track> tracks(m_param_tracksStoreArrayName);
  tracks.registerInDataStore();
  tracks.registerRelationTo(recoTracks);

  StoreArray<MCParticle> mcParticles;
  if (mcParticles.isOptional()) {
    tracks.registerRelationTo(mcParticles);
  }

  StoreArray<TrackFitResult> trackFitResults;
  trackFitResults.registerInDataStore();
  recoTracks.registerRelationTo(trackFitResults);
  trackCands.registerRelationTo(trackFitResults);
}

bool TrackBuilderFromRecoTracksModule::createTrackFitResult(const RecoTrack& recoTrack, Track& newBelleTrack,
                                                            const genfit::AbsTrackRep* trackRep)
{

  // COPY FROM TRACKBUILDER
  const auto pdgCode = trackRep->getPDG();
  if (!Const::chargedStableSet.contains(Const::ParticleType(abs(pdgCode)))) {
    B2DEBUG(100, "Track fitted with hypothesis that is not a ChargedStable (PDG code = " << pdgCode << ")");
    return false;
  }

  bool fitSuccess = recoTrack.hasFitStatus(trackRep);
  genfit::FitStatus* fs = 0;
  if (fitSuccess) {
    fs = recoTrack.getFitStatus(trackRep);
    fitSuccess = fitSuccess && fs->isFitted();
    fitSuccess = fitSuccess && fs->isFitConverged();
    genfit::KalmanFitStatus* kfs = dynamic_cast<genfit::KalmanFitStatus*>(fs);
    fitSuccess = fitSuccess && kfs;
  }

  // Ignore tracks that could not be fitted.
  if (not fitSuccess)
    return false;

  //fill hit patterns VXD and CDC
  HitPatternVXD theHitPatternVXD = TrackBuilderModule::getHitPatternVXD(recoTrack);
  uint32_t hitPatternVXDInitializer = (uint32_t)theHitPatternVXD.getInteger();

  HitPatternCDC theHitPatternCDC = TrackBuilderModule::getHitPatternCDC(recoTrack);
  long long int hitPatternCDCInitializer = (long long int)theHitPatternCDC.getInteger();

  // To calculate the correct starting helix parameters, one
  // has to extrapolate the track to its 'start' (here: take
  // point of closest approach to the origin)

  TVector3 pos(0, 0, 0); //origin
  TVector3 lineDirection(0, 0, 1); // beam axis
  TVector3 poca(0., 0., 0.); //point of closest approach
  TVector3 dirInPoca(0., 0., 0.); //direction of the track at the point of closest approach
  TMatrixDSym cov(6);

  try {
    //extrapolate the track to the origin, the results are stored directly in poca and dirInPoca
    genfit::MeasuredStateOnPlane mop = recoTrack.getFittedState();
    mop.extrapolateToLine(pos, lineDirection);
    mop.getPosMomCov(poca, dirInPoca, cov);

    B2DEBUG(149, "Point of closest approach: " << poca.x() << "  " << poca.y() << "  " << poca.z());
    B2DEBUG(149, "Track direction in POCA: " << dirInPoca.x() << "  " << dirInPoca.y() << "  " << dirInPoca.z());

    int charge = fs->getCharge();
    double pVal = fs->getPVal();
    double Bx, By, Bz;
    genfit::FieldManager::getInstance()->getFieldVal(poca.X(), poca.Y(), poca.Z(),
                                                     Bx, By, Bz);

    StoreArray<TrackFitResult> trackFitResults;
    Const::ChargedStable chargedStable(abs(pdgCode));
    TrackFitResult* newTrackFitResult = trackFitResults.appendNew(poca, dirInPoca, cov, charge, chargedStable,
                                        pVal, Bz / 10.,  hitPatternCDCInitializer, hitPatternVXDInitializer);
    B2DEBUG(100, "Built TrackFitResult for hypothesis PDG = " << pdgCode);

    int tfrIndex = trackFitResults.getEntries() - 1;
    newBelleTrack.setTrackFitResultIndex(chargedStable, tfrIndex);

    // Add all relations
    recoTrack.addRelationTo(newTrackFitResult);
    genfit::TrackCand* relatedTrackCand = recoTrack.getRelated<genfit::TrackCand>(m_param_trackCandidatesStoreArrayName);
    DataStore::addRelationFromTo(relatedTrackCand, newTrackFitResult);

    return true;
  } catch (...) {
    B2WARNING("Something went wrong during the extrapolation of fit results!");
    return false;
  }
}

void TrackBuilderFromRecoTracksModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  StoreArray<Track> tracks(m_param_tracksStoreArrayName);
  tracks.create();

  StoreArray<TrackFitResult> trackFitResults;
  trackFitResults.create();

  for (const RecoTrack& recoTrack : recoTracks) {

    Track newBelleTrack;
    bool hasTrackFitResult = false;

    for (unsigned int representationIndex = 0; representationIndex < recoTrack.getNumReps(); ++representationIndex) {
      const genfit::AbsTrackRep* trackRep = recoTrack.getTrackRep(representationIndex);
      if (createTrackFitResult(recoTrack, newBelleTrack, trackRep)) {
        hasTrackFitResult = true;
      }
    }

    if (hasTrackFitResult) {
      // Create a StoreArray entry from a copy.
      Track* addedBelleTrack = tracks.appendNew(newBelleTrack);

      MCParticle* relatedMCParticle = recoTrack.getRelated<MCParticle>();
      if (relatedMCParticle != nullptr) {
        addedBelleTrack->addRelationTo(relatedMCParticle);
      }

      B2DEBUG(50, "Built Belle2::Track");
    }
  }
}
