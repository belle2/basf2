/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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

#include <TMatrixDSym.h>

#include <genfit/FitStatus.h>
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
  B2DEBUG(27, trackReps.size() << " track representations available.");
  Track newTrack(recoTrack.getQualityIndicator());

  bool repAlreadySet = false;
  unsigned int repIDPlusOne = 0;
  for (const auto& trackRep : trackReps) {
    repIDPlusOne++;

    // Check if the fitted particle type is in our charged stable set.
    const Const::ParticleType particleType(std::abs(trackRep->getPDG()));
    if (not Const::chargedStableSet.contains(particleType)) {
      B2DEBUG(27, "Track fitted with hypothesis that is not a ChargedStable (PDG code = " << particleType.getPDGCode() << ")");
      continue;
    }

    // Check if the fit worked.
    if (not recoTrack.wasFitSuccessful(trackRep)) {
      B2DEBUG(27, "The fit with the given track representation (" << std::abs(trackRep->getPDG()) <<
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
        msop = recoTrack.getMeasuredStateOnPlaneClosestTo(ROOT::Math::XYZVector(0, 0, 0), trackRep);
      } else {
        msop = recoTrack.getMeasuredStateOnPlaneFromFirstHit(trackRep);
      }
    } catch (genfit::Exception& exception) {
      B2WARNING(exception.what());
      continue;
    } catch (const std::runtime_error& er) {
      B2WARNING("Runtime error encountered: " << er.what());
      continue;
    } catch (...) {
      B2WARNING("Undefined exception encountered.");
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
    B2DEBUG(29, "Point of closest approach: " << poca.x() << "  " << poca.y() << "  " << poca.z());
    B2DEBUG(29, "Track direction in POCA: " << dirInPoca.x() << "  " << dirInPoca.y() << "  " << dirInPoca.z());

    const int charge = recoTrack.getTrackFitStatus(trackRep)->getCharge();
    const double pValue = recoTrack.getTrackFitStatus(trackRep)->getPVal();
    const double nDF = recoTrack.getTrackFitStatus(trackRep)->getNdf();

    double Bx, By, Bz;  // In cgs units
    if (useBFieldAtHit) {
      const B2Vector3D& hitPosition = msop.getPos();
      genfit::FieldManager::getInstance()->getFieldVal(hitPosition.X(), hitPosition.Y(), hitPosition.Z(), Bx, By, Bz);
    } else {
      genfit::FieldManager::getInstance()->getFieldVal(poca.X(), poca.Y(), poca.Z(), Bx, By, Bz);
    }
    Bz = Bz / 10.; // In SI-Units

    const uint64_t hitPatternCDCInitializer = getHitPatternCDCInitializer(recoTrack, trackRep);
    const uint32_t hitPatternVXDInitializer = getHitPatternVXDInitializer(recoTrack, trackRep);

    const auto newTrackFitResult = trackFitResults.appendNew(
                                     ROOT::Math::XYZVector(poca), ROOT::Math::XYZVector(dirInPoca), cov, charge, particleType, pValue, Bz,
                                     hitPatternCDCInitializer, hitPatternVXDInitializer, nDF
                                   );

    const int newTrackFitResultArrayIndex = newTrackFitResult->getArrayIndex();
    newTrack.setTrackFitResultIndex(particleType, newTrackFitResultArrayIndex);
  }

  B2DEBUG(27, "Number of fitted hypothesis = " << newTrack.getNumberOfFittedHypotheses());
  if (newTrack.getNumberOfFittedHypotheses() > 0) {
    Track* addedTrack = tracks.appendNew(newTrack);
    addedTrack->addRelationTo(&recoTrack);
    return true;
  } else {
    B2DEBUG(28, "No valid fit for any given hypothesis. No Track is added to the Tracks StoreArray.");
  }
  return true;
}


uint32_t TrackBuilder::getHitPatternVXDInitializer(const RecoTrack& recoTrack, const genfit::AbsTrackRep* representation)
{
  HitPatternVXD hitPatternVXD;

  const auto& hitPointsWithMeasurements = recoTrack.getHitPointsWithMeasurement();
  int nNotFittedVXDhits = 0;

  for (const auto& trackPoint : hitPointsWithMeasurements) {  // Loop on TrackPoint

    genfit::KalmanFitterInfo* kalmanInfo = trackPoint->getKalmanFitterInfo(representation);

    for (size_t measurementId = 0; measurementId < trackPoint->getNumRawMeasurements(); measurementId++) {  //Loop on raw measurement

      genfit::AbsMeasurement* absMeas = trackPoint->getRawMeasurement(measurementId);

      PXDRecoHit* pxdHit = dynamic_cast<PXDRecoHit*>(absMeas);
      SVDRecoHit* svdHit = dynamic_cast<SVDRecoHit*>(absMeas);
      SVDRecoHit2D* svdHit2D = dynamic_cast<SVDRecoHit2D*>(absMeas);

      if (!pxdHit && !svdHit2D && !svdHit)
        continue; // consider only VXD hits

      if (kalmanInfo) {

        if (kalmanInfo->getNumMeasurements() > 1)
          B2WARNING("VXD TrackPoint contains more than one KalmanFitterInfo: only the first will be considered");

        const double weight = kalmanInfo->getWeights().at(0); // only 1st KalmanFitterInfo considered
        if (weight < 1.e-9)
          continue;           // skip kfinfo with negligible weight

        if (pxdHit) {
          const int layerNumber = pxdHit->getSensorID().getLayerNumber();
          const int currentHits = hitPatternVXD.getPXDLayer(layerNumber, HitPatternVXD::PXDMode::normal);
          hitPatternVXD.setPXDLayer(layerNumber, currentHits + 1, HitPatternVXD::PXDMode::normal);
        }

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
            hitPatternVXD.setSVDLayer(layerNumber, currentHits.first, currentHits.second + 1);
        }

      }   // end of if kalmanInfo
      else {
        // i.e. if !kalmanInfo)
        ++nNotFittedVXDhits;    // counts TrackPoints with VXD hits without KalmanFitterInfo
        continue;
      }

    } // end of loop on raw measurements
  } // end of loop on TrackPoint

  if (nNotFittedVXDhits > 0) {
    B2DEBUG(27, " No KalmanFitterInfo associated to some TrackPoints with VXD hits, not filling the HitPatternVXD");
    B2DEBUG(27, nNotFittedVXDhits << " had no FitterInfo");
  }
  return hitPatternVXD.getInteger();
}


uint64_t TrackBuilder::getHitPatternCDCInitializer(const RecoTrack& recoTrack, const genfit::AbsTrackRep* representation)
{
  HitPatternCDC hitPatternCDC;

  int nCDChits = 0;
  int nNotFittedCDChits = 0;

  const auto& hitPointsWithMeasurements = recoTrack.getHitPointsWithMeasurement();

  for (const auto& trackPoint : hitPointsWithMeasurements) { // Loop on TrackPoint

    genfit::KalmanFitterInfo* kalmanInfo = trackPoint->getKalmanFitterInfo(representation);

    for (size_t measurementId = 0; measurementId < trackPoint->getNumRawMeasurements(); measurementId++) { //Loop on raw measurement

      genfit::AbsMeasurement* absMeas = trackPoint->getRawMeasurement(measurementId);
      CDCRecoHit* cdcHit = dynamic_cast<CDCRecoHit*>(absMeas);

      if (!cdcHit)
        continue; // consider only CDC hits

      if (kalmanInfo) {
        bool isValidWeight = kFALSE;
        for (unsigned int kfinfoId = 0; kfinfoId < kalmanInfo->getNumMeasurements(); kfinfoId++) { // Loop on KalmanFitterInfo
          const double weight = kalmanInfo->getWeights().at(kfinfoId);
          if (weight < 1.e-9)
            continue;           // skip kfinfo with negligible weight
          else {
            isValidWeight = kTRUE;
            B2DEBUG(27, "CDC: " << nCDChits << "\t" << cdcHit->getWireID().getEWire() << "\t" << kfinfoId << "\t" << weight);
          }
        }                       // end of KalmanFitterInfo loop

        if (isValidWeight) {    // fill nCDChits only one time per each raw measurement
          WireID wire = cdcHit->getWireID();
          hitPatternCDC.setLayer(wire.getICLayer());
          nCDChits++;             // counts CDC hits where there is KalmanFitterInfo and not negligible weight
        }

      }   // end of if kalmanInfo
      else {
        // i.e. if !kalmanInfo)
        ++nNotFittedCDChits;    // counts TrackPoints with CDC hits without KalmanFitterInfo
        continue;
      }

    } // end of loop on raw measurements
  } // end of loop on TrackPoint

  if (nNotFittedCDChits > 0) {
    B2DEBUG(27, " No KalmanFitterInfo associated to some TrackPoints with CDC hits, not filling the HitPatternCDC");
    B2DEBUG(27, nNotFittedCDChits << " out of " << nCDChits << " had no FitterInfo");
  }
  hitPatternCDC.setNHits(nCDChits);

  return hitPatternCDC.getInteger();

}


