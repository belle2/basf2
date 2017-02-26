/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/genfitter/TrackBuilderModule.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <genfit/FieldManager.h>
#include <genfit/AbsTrackRep.h>
#include <genfit/FitStatus.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/KalmanFitterInfo.h>

using namespace Belle2;

REG_MODULE(TrackBuilder)

TrackBuilderModule::TrackBuilderModule() :
  Module()
{
  setDescription(
    "Build TrackFitResults. Needs genfit::TrackCands and genfit::Tracks as input, creates Belle2::Tracks and Belle2::TrackFitResults as output.");
  setPropertyFlags(c_ParallelProcessingCertified);

  //input
  addParam("GFTrackCandidatesColName", m_gfTrackCandsColName,
           "Name of collection holding the genfit::TrackCandidates (should be "
           "created by the pattern recognition or MCTrackFinderModule).",
           std::string(""));
  addParam("GFTracksColName", m_gfTracksColName,
           "Name of collection holding the final genfit::Tracks (created by GenFitterModule)",
           std::string(""));
  addParam("MCParticlesColName", m_mcParticlesColName,
           "Name of collection holding the MCParticles (need to create relations between found tracks and MCParticles)",
           std::string(""));

  addParam("beamSpot", m_beamSpot,
           "Point on line parallel z to which the fitted track will be "
           "extrapolated in order to put together the TrackFitResults.",
           std::vector<double> {0.0, 0.0, 0.0});
}

TrackBuilderModule::~TrackBuilderModule()
{
}

void TrackBuilderModule::initialize()
{
  B2WARNING("This module is depricated as it uses genfit::Track(Cand)s instead of RecoTracks. It will be removed in the future. If you need information on the transition, please contact Nils Braun (nils.braun@kit.edu).");
  StoreArray<MCParticle> mcParticles(m_mcParticlesColName);
  mcParticles.isOptional();

  StoreArray<genfit::TrackCand> trackCandidates(m_gfTrackCandsColName);
  trackCandidates.isRequired();
  StoreArray<genfit::Track> gftracks(m_gfTracksColName);
  gftracks.isRequired();

  StoreArray<Track> tracks;
  tracks.registerInDataStore();
  StoreArray<TrackFitResult> trackfitresults;
  trackfitresults.registerInDataStore();

  tracks.registerRelationTo(mcParticles);
  gftracks.registerRelationTo(trackfitresults);
  trackCandidates.registerRelationTo(trackfitresults);
}

void TrackBuilderModule::beginRun()
{
}

void TrackBuilderModule::event()
{
  StoreArray < MCParticle > mcParticles(m_mcParticlesColName);
  StoreArray < genfit::TrackCand > trackCandidates(m_gfTrackCandsColName);
  B2DEBUG(99, "TrackBuilder: Number of genfit::TrackCandidates: " << trackCandidates.getEntries());
  if (trackCandidates.getEntries() == 0)
    B2DEBUG(100, "TrackBuilder: genfit::TrackCandidatesCollection is empty!");

  //StoreArrays to store the fit results
  StoreArray < Track > tracks;
  StoreArray < TrackFitResult > trackFitResults;
  StoreArray < genfit::Track > gfTracks(m_gfTracksColName);

  //Relations for Tracks
  RelationArray gfTracksToTrackFitResults(gfTracks, trackFitResults);
  RelationArray gfTrackCandidatesToTrackFitResults(trackCandidates, trackFitResults);

  // Utility used to match everything up
  RelationArray gfTrackCandidatesTogfTracks(trackCandidates, gfTracks);

  for (int iGFTrack = 0; iGFTrack < gfTracks.getEntries(); ++iGFTrack) {
    assert(gfTracks[iGFTrack]);
    const genfit::Track* gfTrack = gfTracks[iGFTrack];

    const genfit::TrackCand* trackCand = DataStore::getRelated<genfit::TrackCand>(gfTrack, m_gfTrackCandsColName);
    if (!trackCand) {
      B2ERROR("No genfit::TrackCand for genfit::Track");
      continue;
    }
    // We need to know the index of the TrackCand in the StoreArray in
    // order to match the TrackFitResults and Tracks to the
    // TrackCand.  Since they are not RelationsObjects, we have to do
    // it in this ugly way.
    int iTrackCand = -1;
    for (int i = 0; i < trackCandidates.getEntries(); ++i) {
      if (trackCand == trackCandidates[i]) {
        iTrackCand = i;
        break;
      }
    }
    if (iTrackCand == -1) {
      B2ERROR("No StoreArray entry corresponding to TrackCand.");
      continue;
    }

    // We create an empty track.  If, in the end, we manage to fill
    // any TrackFitResults for it, we put a copy into the StoreArray.
    Track newTrack;
    bool haveTFR = false;

    for (unsigned int iRep = 0; iRep < gfTrack->getNumReps(); ++iRep) {
      const genfit::AbsTrackRep* trackRep = gfTrack->getTrackRep(iRep);
      int iPDG = trackRep->getPDG();
      if (!Const::chargedStableSet.contains(Const::ParticleType(abs(iPDG)))) {
        B2DEBUG(100,
                "Track fitted with hypothesis that is not a ChargedStable (PDG code = "
                << iPDG << ")");
        continue;
      }

      Const::ChargedStable chargedStable(abs(iPDG));

      bool fitSuccess = gfTrack->hasFitStatus(trackRep);
      genfit::FitStatus* fs = 0;
      genfit::KalmanFitStatus* kfs = 0;
      if (fitSuccess) {
        fs = gfTrack->getFitStatus(trackRep);
        fitSuccess = fitSuccess && fs->isFitted();
        fitSuccess = fitSuccess && fs->isFitConverged();
        kfs = dynamic_cast<genfit::KalmanFitStatus*>(fs);
        fitSuccess = fitSuccess && kfs;
      }

      // Ignore tracks that could not be fitted.
      if (!fitSuccess)
        continue;

      //fill hit patterns VXD and CDC
      HitPatternVXD theHitPatternVXD =  getHitPatternVXD(*gfTrack);
      uint32_t hitPatternVXDInitializer = (uint32_t)theHitPatternVXD.getInteger();

      HitPatternCDC theHitPatternCDC =  getHitPatternCDC(*gfTrack);
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
        genfit::MeasuredStateOnPlane mop = gfTrack->getFittedState(iRep);
        mop.extrapolateToLine(pos, lineDirection);
        mop.getPosMomCov(poca, dirInPoca, cov);

        B2DEBUG(149, "Point of closest approach: " << poca.x() << "  " << poca.y() << "  " << poca.z());
        B2DEBUG(149, "Track direction in POCA: " << dirInPoca.x() << "  " << dirInPoca.y() << "  " << dirInPoca.z());

        //Create relations
        //if (aTrackCandPointer->getMcTrackId() >= 0) {
        //tracksToMcParticles.add(trackCounter, aTrackCandPointer->getMcTrackId());
        //}

        int charge = fs->getCharge();
        double pVal = fs->getPVal();
        double Bx, By, Bz;
        genfit::FieldManager::getInstance()->getFieldVal(poca.X(), poca.Y(), poca.Z(),
                                                         Bx, By, Bz);
        trackFitResults.appendNew(poca, dirInPoca, cov, charge, chargedStable,
                                  pVal, Bz / 10.,  hitPatternCDCInitializer, hitPatternVXDInitializer);
        B2DEBUG(100, "Built TrackFitResult for hypothesis PDG = " << iPDG);

        int tfrIndex = trackFitResults.getEntries() - 1;
        newTrack.setTrackFitResultIndex(chargedStable, tfrIndex);
        haveTFR = true;

        gfTracksToTrackFitResults.add(iGFTrack, tfrIndex);
        gfTrackCandidatesToTrackFitResults.add(iTrackCand, tfrIndex);
      } catch (...) {
        B2WARNING("Something went wrong during the extrapolation of fit results!");
      }

    }

    if (haveTFR) {
      // Create a StoreArray entry from a copy.
      Track* tr = tracks.appendNew(newTrack);

      B2DEBUG(50, "Built Belle2::Track");
      // Do MC association.
      if (mcParticles.getEntries() > 0) {
        MCParticle* mcparticle = DataStore::Instance().getRelated<MCParticle>(gfTrack);
        // relationToMCPart can be a nullptr (as of 2015-05-08 this
        // happens in framework/tests/streamer_test.py), so we have to
        // guard against this.  Otherwise we check if the relation
        // points to an MC particle and if so, we build the relation.
        if (mcparticle) {
          tr->addRelationTo(mcparticle);
          B2DEBUG(100, "Associated new Belle2::Track with Belle2::MCParticle");
        }
      }
    }
  }
}


void TrackBuilderModule::endRun()
{
}


void TrackBuilderModule::terminate()
{
}


HitPatternVXD TrackBuilderModule::getHitPatternVXD(const genfit::Track& track)
{

  Int_t PXD_Hits[2] = {0, 0};
  Int_t SVD_uHits[4] = {0, 0, 0, 0};
  Int_t SVD_vHits[4] = {0, 0, 0, 0};

  HitPatternVXD aHitPatternVXD;

  //hits used in the fit
  int nHits = track.getNumPointsWithMeasurement();
  int nNotFittedVXDhits = 0;

  for (int i = 0; i < nHits; i++) {
    genfit::TrackPoint* tp = track.getPointWithMeasurement(i);

    int nMea = tp->getNumRawMeasurements();
    for (int mea = 0; mea < nMea; mea++) {

      genfit::AbsMeasurement* absMeas = tp->getRawMeasurement(mea);

      double weight = 0;
      std::vector<double> weights;
      genfit::KalmanFitterInfo* kalmanInfo = tp->getKalmanFitterInfo();
      if (kalmanInfo) {
        weights = kalmanInfo->getWeights();
        weight = weights.at(mea);
      } else {
        ++nNotFittedVXDhits;
        continue;
      }

      if (weight == 0)
        continue;

      PXDRecoHit* pxdHit =  dynamic_cast<PXDRecoHit*>(absMeas);
      SVDRecoHit2D* svdHit2D =  dynamic_cast<SVDRecoHit2D*>(absMeas);
      SVDRecoHit* svdHit =  dynamic_cast<SVDRecoHit*>(absMeas);

      if (pxdHit) {
        VxdID sensor = pxdHit->getSensorID();
        if (sensor.getLayerNumber() > 2)
          B2WARNING("wrong PXD layer (>2)");
        PXD_Hits[ sensor.getLayerNumber() - 1 ]++;
      } else if (svdHit2D) {

        //  B2WARNING("No way to handle 2D SVD hits (fill HitPatternVXD)");
        VxdID sensor = svdHit2D->getSensorID();
        if (sensor.getLayerNumber() < 2 ||  sensor.getLayerNumber() > 6)
          B2WARNING("wrong SVD layer (<2 || >6)");
        SVD_uHits[ sensor.getLayerNumber() - 3]++;
        SVD_vHits[ sensor.getLayerNumber() - 3]++;
      } else if (svdHit) {
        VxdID sensor = svdHit->getSensorID();
        if (sensor.getLayerNumber() < 2 ||  sensor.getLayerNumber() > 6)
          B2WARNING("wrong SVD layer (<2 || >6)");
        if (svdHit->isU())
          SVD_uHits[ sensor.getLayerNumber() - 3]++;
        else
          SVD_vHits[ sensor.getLayerNumber() - 3]++;
      }
    }
  }

  //fill PXD hits
  for (int layerId = 1; layerId <= 2; ++layerId) {
    //maximum number of hits checked inside the HitPatternVXD
    int arrayId = layerId - 1;
    aHitPatternVXD.setPXDLayer(layerId, PXD_Hits[arrayId], HitPatternVXD::PXDMode::normal); //normal/gated mode not retireved
  }

  //fill SVD hits
  for (int layerId = 3; layerId <= 6; ++layerId) {
    //maximum number of hits checked inside the HitPatternVXD
    int arrayId = layerId - 3;
    aHitPatternVXD.setSVDLayer(layerId, SVD_uHits[arrayId], SVD_vHits[arrayId]);
  }

  if (nNotFittedVXDhits > 0) {
    B2DEBUG(100, " No KalmanFitterInfo associated to some TrackPoints with VXD hits, not filling the HitPatternVXD");
    B2DEBUG(100, nNotFittedVXDhits << " had no FitterInfo");
  }
  return aHitPatternVXD;
}

HitPatternCDC TrackBuilderModule::getHitPatternCDC(const genfit::Track& track)
{

  HitPatternCDC aHitPatternCDC(0);

  //hits used in the fit
  int nHits = track.getNumPointsWithMeasurement();
  int nCDChits = 0;
  int nNotFittedCDChits = 0;
  for (int i = 0; i < nHits; i++) {
    genfit::TrackPoint* tp = track.getPointWithMeasurement(i);

    int nMea = tp->getNumRawMeasurements();
    for (int mea = 0; mea < nMea; mea++) {

      genfit::AbsMeasurement* absMeas = tp->getRawMeasurement(mea);

      double weight = 0;
      std::vector<double> weights;
      genfit::KalmanFitterInfo* kalmanInfo = tp->getKalmanFitterInfo();

      if (kalmanInfo) {
        weights = kalmanInfo->getWeights();
        weight = weights.at(mea);
      } else {
        ++nNotFittedCDChits;
        continue;
      }

      if (weight == 0)
        continue;

      CDCRecoHit* cdcHit =  dynamic_cast<CDCRecoHit*>(absMeas);

      if (cdcHit) {
        WireID wire = cdcHit->getWireID();

        //maximum number of hits checked inside the HitPatternCDC
        aHitPatternCDC.setLayer(wire.getICLayer());
        nCDChits++;
      }
    }

  }
  if (nNotFittedCDChits > 0) {
    B2DEBUG(100, " No KalmanFitterInfo associated to some TrackPoints with CDC hits, not filling the HitPatternCDC");
    B2DEBUG(100, nNotFittedCDChits << " out of " << nCDChits << " had no FitterInfo");
  }
  aHitPatternCDC.setNHits(nCDChits);

  return aHitPatternCDC;

}
