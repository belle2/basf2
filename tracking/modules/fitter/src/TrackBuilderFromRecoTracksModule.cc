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

using namespace std;
using namespace Belle2;

REG_MODULE(TrackBuilderFromRecoTracks)

TrackBuilderFromRecoTracksModule::TrackBuilderFromRecoTracksModule() :
  Module()
{
  setDescription("Create Belle::tracks from the given reco Tracks.");

  addParam("RecoTracksStoreArrayName", m_param_recoTracksStoreArrayName, "StoreArray name of the input and output reco tracks.",
           std::string("RecoTracks"));

  addParam("TracksStoreArrayName", m_param_tracksStoreArrayName, "StoreArray name of the output genfit::tracks.",
           std::string("Tracks"));

}

void TrackBuilderFromRecoTracksModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  StoreArray<Track> tracks(m_param_tracksStoreArrayName);
  tracks.registerInDataStore();

  tracks.registerRelationTo(recoTracks);

  StoreArray<TrackFitResult> trackFitResults;
  trackFitResults.registerInDataStore();

  recoTracks.registerRelationTo(trackFitResults);
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

    // COPY FROM TRACKBUILDER
    for (unsigned int iRep = 0; iRep < recoTrack.getNumReps(); ++iRep) {
      const auto trackRep = recoTrack.getTrackRep(iRep);
      const auto pdgCode = trackRep->getPDG();
      if (!Const::chargedStableSet.contains(Const::ParticleType(abs(pdgCode)))) {
        B2DEBUG(100, "Track fitted with hypothesis that is not a ChargedStable (PDG code = " << pdgCode << ")");
        continue;
      }
      Const::ChargedStable chargedStable(abs(pdgCode));

      bool fitSuccess = recoTrack.hasFitStatus(trackRep);
      genfit::FitStatus* fs = 0;
      genfit::KalmanFitStatus* kfs = 0;
      if (fitSuccess) {
        fs = recoTrack.getFitStatus(trackRep);
        fitSuccess = fitSuccess && fs->isFitted();
        fitSuccess = fitSuccess && fs->isFitConverged();
        kfs = dynamic_cast<genfit::KalmanFitStatus*>(fs);
        fitSuccess = fitSuccess && kfs;
      }

      // Ignore tracks that could not be fitted.
      if (!fitSuccess)
        continue;

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
        TrackFitResult* newTrackFitResult = trackFitResults.appendNew(poca, dirInPoca, cov, charge, chargedStable,
                                            pVal, Bz / 10.,  hitPatternCDCInitializer, hitPatternVXDInitializer);
        B2DEBUG(100, "Built TrackFitResult for hypothesis PDG = " << pdgCode);

        int tfrIndex = trackFitResults.getEntries() - 1;
        newBelleTrack.setTrackFitResultIndex(chargedStable, tfrIndex);
        hasTrackFitResult = true;

        recoTrack.addRelationTo(newTrackFitResult);
      } catch (...) {
        B2WARNING("Something went wrong during the extrapolation of fit results!");
      }
    }

    if (hasTrackFitResult) {
      // Create a StoreArray entry from a copy.
      Track* track = tracks.appendNew(newBelleTrack);

      B2DEBUG(50, "Built Belle2::Track");
    }
  }
}

/*
 * for (const auto& trackCandidate : trackCandidates) {
    const auto gfTracks = DataStore::getRelationsFromObj<genfit::Track>(&trackCandidate, m_gfTrackCandsColName);

    if (gfTracks.size() == 0) {
      B2DEBUG(100, "No genfit::Track for genfit::TrackCand");
      continue;
    }
    // We need to know the index of the TrackCand in the StoreArray in
    // order to match the TrackFitResults and Tracks to the
    // TrackCand.  Since they are not RelationsObjects, we have to do
    // it in this ugly way.
    int iTrackCand = -1;
    for (int i = 0; i < trackCandidates.getEntries(); ++i) {
      if (&trackCandidate == trackCandidates[i]) {
        iTrackCand = i;
        break;
      }
    }
    if (iTrackCand == -1) {
      B2ERROR("No StoreArray entry corresponding to TrackCand.");
      continue;
    }

    Track newTrack;
    bool haveTFR = false;

    for (const auto& gfTrack : gfTracks) {

      int iGFTrack = -1;
      for (unsigned int i = 0; i < gfTracks.size(); ++i) {
        if (&gfTrack == gfTracks[i]) {
          iGFTrack = i;
          break;
        }
      }
      if (iGFTrack == -1) {
        B2ERROR("No StoreArray entry corresponding to GFTrack.");
        continue;
      }

      for (unsigned int iRep = 0; iRep < gfTrack.getNumReps(); ++iRep) {
        const auto trackRep = gfTrack.getTrackRep(iRep);
        const auto pdgCode = trackRep->getPDG();
        if (!Const::chargedStableSet.contains(Const::ParticleType(abs(pdgCode)))) {
          B2DEBUG(100, "Track fitted with hypothesis that is not a ChargedStable (PDG code = " << pdgCode << ")");
          continue;
        }
        Const::ChargedStable chargedStable(abs(pdgCode));

        bool fitSuccess = gfTrack.hasFitStatus(trackRep);
        genfit::FitStatus* fs = 0;
        genfit::KalmanFitStatus* kfs = 0;
        if (fitSuccess) {
          fs = gfTrack.getFitStatus(trackRep);
          fitSuccess = fitSuccess && fs->isFitted();
          fitSuccess = fitSuccess && fs->isFitConverged();
          kfs = dynamic_cast<genfit::KalmanFitStatus*>(fs);
          fitSuccess = fitSuccess && kfs;
        }

        // Ignore tracks that could not be fitted.
        if (!fitSuccess)
          continue;

        //fill hit patterns VXD and CDC
        HitPatternVXD theHitPatternVXD =  getHitPatternVXD(gfTrack);
        uint32_t hitPatternVXDInitializer = (uint32_t)theHitPatternVXD.getInteger();

        HitPatternCDC theHitPatternCDC =  getHitPatternCDC(gfTrack);
        long long int hitPatternCDCInitializer = (long long int)theHitPatternCDC.getInteger();

        // To calculate the correct starting helix parameters, one
        // has to extrapolate the track to its 'start' (here: take
        // point of closest approach to the origin)

        TVector3 pos(m_beamSpot.at(0), m_beamSpot.at(1), m_beamSpot.at(2)); //origin
        TVector3 lineDirection(0, 0, 1); // beam axis
        TVector3 poca(0., 0., 0.); //point of closest approach
        TVector3 dirInPoca(0., 0., 0.); //direction of the track at the point of closest approach
        TMatrixDSym cov(6);

        try {
          //extrapolate the track to the origin, the results are stored directly in poca and dirInPoca
          genfit::MeasuredStateOnPlane mop = gfTrack.getFittedState();
          mop.extrapolateToLine(pos, lineDirection);
          mop.getPosMomCov(poca, dirInPoca, cov);

          B2DEBUG(149, "Point of closest approach: " << poca.x() << "  " << poca.y() << "  " << poca.z());
          B2DEBUG(149, "Track direction in POCA: " << dirInPoca.x() << "  " << dirInPoca.y() << "  " << dirInPoca.z());

          int charge = fs->getCharge();
          double pVal = fs->getPVal();
          double Bx, By, Bz;
          genfit::FieldManager::getInstance()->getFieldVal(poca.X(), poca.Y(), poca.Z(),
                                                           Bx, By, Bz);
          trackFitResults.appendNew(poca, dirInPoca, cov, charge, chargedStable,
                                    pVal, Bz / 10.,  hitPatternCDCInitializer, hitPatternVXDInitializer);
          B2DEBUG(100, "Built TrackFitResult for hypothesis PDG = " << pdgCode);

          int tfrIndex = trackFitResults.getEntries() - 1;
          newTrack.setTrackFitResultIndex(chargedStable, tfrIndex);
          haveTFR = true;

          gfTracksToTrackFitResults.add(iGFTrack, tfrIndex);
          gfTrackCandidatesToTrackFitResults.add(iTrackCand, tfrIndex);
        } catch (...) {
          B2WARNING("Something went wrong during the extrapolation of fit results!");
        }
      }
    }
    if (haveTFR) {
      // Create a StoreArray entry from a copy.
      Track* tr = tracks.appendNew(newTrack);

      B2DEBUG(50, "Built Belle2::Track");
      //Do MC association.
      if (mcParticles.getEntries() > 0) {
        auto relationToMCPart = gfTrackCandsToMCPart.getFirstElementFrom(trackCandidate);
        // relationToMCPart can be a nullptr (as of 2015-05-08 this
        // happens in framework/tests/streamer_test.py), so we have to
        // guard against this.  Otherwise we check if the relation
        // points to an MC particle and if so, we build the relation.
        if (relationToMCPart && relationToMCPart->to) {
          tr->addRelationTo(relationToMCPart->to);
          B2DEBUG(100, "Associated new Belle2::Track with Belle2::MCParticle");
        }

      }
    }
  }
 */
