#include <tracking/trackFitting/trackBuilder/factories/TrackBuilder.h>


#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

#include <TVector3.h>
#include <TMatrixDSym.h>

#include <genfit/FitStatus.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/Track.h>
#include <genfit/TrackPoint.h>
#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/FieldManager.h>

using namespace Belle2;

bool TrackBuilder::storeTrackFromRecoTrack(RecoTrack& recoTrack,
                                           const bool useClosestHitToIP, const bool useBFieldAtHit)
{
  StoreArray<Track> tracks(m_trackColName);
  StoreArray<TrackFitResult> trackFitResults(m_trackFitResultColName);

  const auto& trackReps = recoTrack.getRepresentations();
  B2DEBUG(100, trackReps.size() << " track representations available.");
  Track newTrack(recoTrack.getQualityIndicator());

  bool repAlreadySet = false;
  unsigned int repIDPlusOne = 0;
  for (const auto& trackRep : trackReps) {
    repIDPlusOne++;

    // Check if the fitted particle type is in our charged stable set.
    const Const::ParticleType particleType(std::abs(trackRep->getPDG()));
    if (not Const::chargedStableSet.contains(particleType)) {
      B2DEBUG(100, "Track fitted with hypothesis that is not a ChargedStable (PDG code = " << particleType.getPDGCode() << ")");
      continue;
    }

    // Check if the fit worked.
    if (not recoTrack.wasFitSuccessful(trackRep)) {
      B2DEBUG(100, "The fit with the given track representation (" << std::abs(trackRep->getPDG()) <<
              ") was not successful. Skipping ...");
      continue;
    }

    if (not repAlreadySet) {
      RecoTrackGenfitAccess::getGenfitTrack(recoTrack).setCardinalRep(repIDPlusOne - 1);
      repAlreadySet = true;
    }

    // Extrapolate the tracks to the perigee.
    genfit::MeasuredStateOnPlane msop;
    try {
      if (useClosestHitToIP) {
        msop = recoTrack.getMeasuredStateOnPlaneClosestTo(TVector3(0, 0, 0), trackRep);
      } else {
        msop = recoTrack.getMeasuredStateOnPlaneFromFirstHit(trackRep);
      }
    } catch (genfit::Exception& exception) {
      B2WARNING(exception.what());
      continue;
    }

    genfit::MeasuredStateOnPlane extrapolatedMSoP = msop;
    try {
      extrapolatedMSoP.extrapolateToLine(m_beamSpot, m_beamAxis);
    } catch (...) {
      B2WARNING("Could not extrapolate the fit result for pdg " << particleType.getPDGCode() <<
                " to the perigee point. Why, I don't know.");
      continue;
    }

    // Build track fit result.

    TVector3 poca(0., 0., 0.);
    TVector3 dirInPoca(0., 0., 0.);
    TMatrixDSym cov(6);
    extrapolatedMSoP.getPosMomCov(poca, dirInPoca, cov);
    B2DEBUG(149, "Point of closest approach: " << poca.x() << "  " << poca.y() << "  " << poca.z());
    B2DEBUG(149, "Track direction in POCA: " << dirInPoca.x() << "  " << dirInPoca.y() << "  " << dirInPoca.z());

    const int charge = recoTrack.getTrackFitStatus(trackRep)->getCharge();
    const double pValue = recoTrack.getTrackFitStatus(trackRep)->getPVal();

    double Bx, By, Bz;  // In cgs units
    if (useBFieldAtHit) {
      const TVector3& hitPosition = msop.getPos();
      genfit::FieldManager::getInstance()->getFieldVal(hitPosition.X(), hitPosition.Y(), hitPosition.Z(), Bx, By, Bz);
    } else {
      genfit::FieldManager::getInstance()->getFieldVal(poca.X(), poca.Y(), poca.Z(), Bx, By, Bz);
    }
    Bz = Bz / 10.; // In SI-Units

    const uint64_t hitPatternCDCInitializer = getHitPatternCDCInitializer(recoTrack);
    const uint32_t hitPatternVXDInitializer = getHitPatternVXDInitializer(recoTrack);

    const auto newTrackFitResult = trackFitResults.appendNew(
                                     poca, dirInPoca, cov, charge, particleType, pValue, Bz,
                                     hitPatternCDCInitializer, hitPatternVXDInitializer
                                   );

    const int newTrackFitResultArrayIndex = newTrackFitResult->getArrayIndex();
    newTrack.setTrackFitResultIndex(particleType, newTrackFitResultArrayIndex);
  }

  B2DEBUG(100, "Number of fitted hypothesis = " << newTrack.getNumberOfFittedHypotheses());
  if (newTrack.getNumberOfFittedHypotheses() > 0) {
    Track* addedTrack = tracks.appendNew(newTrack);
    addedTrack->addRelationTo(&recoTrack);
    const auto& mcParticleWithWeight = recoTrack.getRelatedToWithWeight<MCParticle>(m_mcParticleColName);
    const MCParticle* mcParticle = mcParticleWithWeight.first;
    if (mcParticle) {
      B2DEBUG(200, "Relation to MCParticle set.");
      addedTrack->addRelationTo(mcParticle, mcParticleWithWeight.second);
    } else {
      B2DEBUG(200, "Relation to MCParticle not set. No related MCParticle to RecoTrack.");
    }
    // false positive due to new with placement (cppcheck issue #7163)
    // cppcheck-suppress memleak
    return true;
  } else {
    B2DEBUG(200, "Relation to MCParticle not set. No related MCParticle to RecoTrack.");
  }
  // false positive due to new with placement (cppcheck issue #7163)
  // cppcheck-suppress memleak
  return true;
}


uint32_t TrackBuilder::getHitPatternVXDInitializer(const RecoTrack& recoTrack) const
{
  HitPatternVXD hitPatternVXD;

  const auto& hitPointsWithMeasurements = recoTrack.getHitPointsWithMeasurement();
  int nNotFittedVXDhits = 0;

  for (const auto& trackPoint : hitPointsWithMeasurements) {

    for (size_t measurementId = 0; measurementId < trackPoint->getNumRawMeasurements(); measurementId++) {

      genfit::AbsMeasurement* absMeas = trackPoint->getRawMeasurement(measurementId);
      genfit::KalmanFitterInfo* kalmanInfo = trackPoint->getKalmanFitterInfo();

      if (kalmanInfo) {
        const double weight = kalmanInfo->getWeights().at(measurementId);
        if (weight == 0)
          continue;
      } else {
        ++nNotFittedVXDhits;
        continue;
      }

      PXDRecoHit* pxdHit = dynamic_cast<PXDRecoHit*>(absMeas);
      if (pxdHit) {
        const int layerNumber = pxdHit->getSensorID().getLayerNumber();
        const int currentHits = hitPatternVXD.getPXDLayer(layerNumber, HitPatternVXD::PXDMode::normal);
        hitPatternVXD.setPXDLayer(layerNumber, currentHits + 1, HitPatternVXD::PXDMode::normal);
      }

      SVDRecoHit* svdHit = dynamic_cast<SVDRecoHit*>(absMeas);
      SVDRecoHit2D* svdHit2D = dynamic_cast<SVDRecoHit2D*>(absMeas);
      if (svdHit2D) {
        const int layerNumber = svdHit2D->getSensorID().getLayerNumber();
        const auto& currentHits = hitPatternVXD.getSVDLayer(layerNumber);
        hitPatternVXD.setSVDLayer(layerNumber, currentHits.first + 1, currentHits.second + 1);
      } else if (svdHit) {
        const int layerNumber = svdHit->getSensorID().getLayerNumber();
        const auto& currentHits = hitPatternVXD.getSVDLayer(layerNumber);

        if (svdHit->isU())
          hitPatternVXD.setSVDLayer(layerNumber, currentHits.first + 1, currentHits.second);
        else
          hitPatternVXD.setSVDLayer(layerNumber, currentHits.first , currentHits.second + 1);
      }

    }
  }

  if (nNotFittedVXDhits > 0) {
    B2DEBUG(100, " No KalmanFitterInfo associated to some TrackPoints with VXD hits, not filling the HitPatternVXD");
    B2DEBUG(100, nNotFittedVXDhits << " had no FitterInfo");
  }
  return hitPatternVXD.getInteger();
}


uint64_t TrackBuilder::getHitPatternCDCInitializer(const RecoTrack& recoTrack) const
{
  HitPatternCDC hitPatternCDC;

  int nCDChits = 0;
  int nNotFittedCDChits = 0;

  const auto& hitPointsWithMeasurements = recoTrack.getHitPointsWithMeasurement();

  for (const auto& trackPoint : hitPointsWithMeasurements) {

    for (size_t measurementId = 0; measurementId < trackPoint->getNumRawMeasurements(); measurementId++) {

      genfit::AbsMeasurement* absMeas = trackPoint->getRawMeasurement(measurementId);
      genfit::KalmanFitterInfo* kalmanInfo = trackPoint->getKalmanFitterInfo();

      if (kalmanInfo) {
        const double weight = kalmanInfo->getWeights().at(measurementId);
        if (weight == 0)
          continue;
      } else {
        ++nNotFittedCDChits;
        continue;
      }

      CDCRecoHit* cdcHit = dynamic_cast<CDCRecoHit*>(absMeas);

      if (cdcHit) {
        WireID wire = cdcHit->getWireID();
        hitPatternCDC.setLayer(wire.getICLayer());
        nCDChits++;
      }
    }

  }
  if (nNotFittedCDChits > 0) {
    B2DEBUG(100, " No KalmanFitterInfo associated to some TrackPoints with CDC hits, not filling the HitPatternCDC");
    B2DEBUG(100, nNotFittedCDChits << " out of " << nCDChits << " had no FitterInfo");
  }
  hitPatternCDC.setNHits(nCDChits);

  return hitPatternCDC.getInteger();

}


