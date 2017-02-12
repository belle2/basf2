/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ian J. Watson                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcToVXDExtrapolator/CDCToVXDExtrapolatorModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <framework/geometry/B2Vector3.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationVector.h>
#include <framework/datastore/RelationArray.h>

#include <tracking/gfbfield/GFGeant4Field.h>
#include <genfit/FieldManager.h>

#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <genfit/RKTrackRep.h>
#include <genfit/AbsKalmanFitter.h>
#include <genfit/KalmanFitter.h>
#include <genfit/DAF.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/KalmanFitterRefTrack.h>
#include <genfit/FitStatus.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/Exception.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>

#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>
#include <cdc/dataobjects/CDCHit.h>

#include <vxd/geometry/GeoCache.h>

#include "TFile.h"

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCToVXDExtrapolator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCToVXDExtrapolatorModule::CDCToVXDExtrapolatorModule() :
  Module(),
  m_saveInfo(false),
  nTotalTracks(0),
  nCdcRecoOnlyTracks(0),
  nCdcRecoOnlyTracksWithVxdMcHits(0),
  nTracksWithAddedHits(0),
  nTracksWithAddedPxdHits(0),
  nStoredTracks(0),
  HitInfo(0),
  TrackInfo(0),
  m_outputFile(0),
  cdcHitsFromMC(0),
  svdClustersFromMC(0),
  pxdClustersFromMC(0)
{
  // Set module properties
  setDescription("For the given track collection input, finds the set of CDC-only tracks, tries to find VXD hits compatible with the tracks.");

  // Parameter definitions
  addParam("SaveAllTracks", m_saveAllTracks,
           "Save tracks which didn't need VXD hit recovery. If false, only those tracks for which hits were found and the refit was successful are saved.",
           true);
  addParam("RootOutputFilename", m_rootOutputFilename,
           "Filename for the ROOT file of module information. If \"\" then won't output ROOT file.", std::string(""));
  addParam("RefitMcHits", m_refitMcHits, "[DEBUG] Refit the CDC-only tracks with the MC particle VXD hits for debugging purposes.",
           false);
  addParam("UseKalman", m_useKalman, "true - refit with Kalman, false - refit with DAF", false);
  addParam("SearchSensorDimensions", m_searchSensorDimensions,
           "true - look for hits within a sensor width/length, false - search within n sigma of the track extrap.", true);
  addParam("HitNSigmaXY", m_hitNSigmaXY,
           "When searching within track extrap. this is the number of sigma a hit needs to be within to be acceptable", float(5.));
  addParam("HitNSigmaZ", m_hitNSigmaZ,
           "When searching within track extrap. this is the number of sigma a hit needs to be within to be acceptable", float(5.));
  addParam("ExtrapolateToDetector", m_extrapolateToDetector,
           "true: will try to extrapolate to the detector plane, false: extrapolates to the abstract cylinder or cone representing the detector",
           false);
  addParam("AllLayers", m_allLayers,
           "true: will try to extrapolate to all layers and find hits, false: stop extrapolation when a layer without compatible hits reached",
           false);
  addParam("ExtrapolateToPxd", m_extrapolateToPxd, "Extrapolate to the PXD as well as the SVD", true);
  addParam("HitNSigmaPix", m_hitNSigmaPix,
           "When searching within track extrap. this is the number of sigma a hit needs to be within to be acceptable (for PXD)", float(5.));
  addParam("StepwiseKalmanRefit", m_stepwiseKalman,
           "When true refits the track with a Kalman update after each hit is added, rather than simply doing the refit at the end. Shoudl improve the extrapolation quality, particularly to the PXD.",
           true);

  // Input
  addParam("GFTrackColName", m_GFTrackColName, "Name of genfit::Track collection (input)", std::string(""));
  addParam("McParticlesColName", m_mcParticlesColName, "Name of MC particles collections", std::string(""));

  // Output
  addParam("OutGFTrackColName", m_OutGFTrackColName, "Name of genfit::Track collection (output)", std::string(""));
  addParam("OutGFTrackCandColName", m_OutGFTrackCandColName, "Name of genfit::TrackCand collection (output)", std::string(""));
}

// account for trapezoidal layers
static inline TVector3 getGlobalPosition(double u, double v, const  VXD::SensorInfoBase& aSensorInfo)
{
  if ((aSensorInfo.getBackwardWidth() > aSensorInfo.getForwardWidth()) == true) { // isWedgeSensor
    double uWedged = (aSensorInfo.getWidth(v) / aSensorInfo.getWidth()) * u;
    return aSensorInfo.pointToGlobal(TVector3(uWedged, v, 0));
  } else {
    return aSensorInfo.pointToGlobal(TVector3(u, v, 0));
  }
}

static inline double deltaXYDist(TVector3& posAtLayer, TVector3& oth)
{
  return fabs(posAtLayer.DeltaPhi(oth));
}

static inline double errDeltaXYDist(TVector3& posAtLayer, TMatrixDSym& covAtLayer)
{
  return ((1. / (posAtLayer.X() * posAtLayer.X() + posAtLayer.Y() * posAtLayer.Y())) *
          sqrt(pow(posAtLayer.Y(), 2) * covAtLayer[0][0] +
               pow(posAtLayer.X(), 2) * covAtLayer[1][1] - 2 * (posAtLayer.Y()) * (posAtLayer.X()) * covAtLayer[0][1]));
};

static inline double errDeltaXYDistSVD(TVector3& posAtLayer, TMatrixDSym& covAtLayer, TVector3& othErr)
{
  return ((1. / (posAtLayer.X() * posAtLayer.X() + posAtLayer.Y() * posAtLayer.Y())) *
          sqrt(pow(posAtLayer.Y(), 2) * (covAtLayer[0][0] + othErr.X() * othErr.X()) +
               pow(posAtLayer.X(), 2) * (covAtLayer[1][1] + othErr.Y() * othErr.Y())
               - 2 * (posAtLayer.Y()) * (posAtLayer.X()) * covAtLayer[0][1]));
};


static bool extrapolateToSensor(SVDCluster* scluster, PXDCluster* pcluster, genfit::Track* track, TVector3& posAtLayer,
                                TVector3& momAtLayer, TMatrixDSym& covAtLayer)
{
  // assumes the track is ordered correctly
  genfit::MeasuredStateOnPlane mop;
  try {
    mop = track->getFittedState(0);
  } catch (...) {
    B2WARNING("Track without any fitted states!");
    return false;
  }
  // need to check the extrapolation worked?
  mop.getPosMomCov(posAtLayer, momAtLayer, covAtLayer);
  try {
    if (scluster) {
      SVDRecoHit rhit(scluster);
      genfit::SharedPlanePtr plane = rhit.constructPlane(mop);
      mop.extrapolateToPlane(plane);
    } else {
      PXDRecoHit rhit(pcluster);
      genfit::SharedPlanePtr plane = rhit.constructPlane(mop);
      mop.extrapolateToPlane(plane);
    }
  } catch (...) {
    B2WARNING("Planar extrapolation failed! continuing");
    // extrapolation failed
    return false;
  }
  B2DEBUG(171, "<-----> SensorExtrap: Old posAtLayer " << posAtLayer.X() << " " << posAtLayer.Y() << " " << posAtLayer.Z());
  mop.getPosMomCov(posAtLayer, momAtLayer, covAtLayer);
  B2DEBUG(171, "<-----> SensorExtrap: New posAtLayer " << posAtLayer.X() << " " << posAtLayer.Y() << " " << posAtLayer.Z());
  return true;
}

static bool extrapolateToCylinder(float radius, genfit::Track* track, TVector3& posAtLayer, TVector3& momAtLayer,
                                  TMatrixDSym& covAtLayer)
{
  static const TVector3 origin(0, 0, 0);
  static const TVector3 zaxis(0, 0, 1);

  // assumes the track is ordered correctly
  genfit::MeasuredStateOnPlane mop;
  try {
    mop = track->getFittedState(0);
  } catch (...) {
    B2WARNING("Track without any fitted states!");
    return false;
  }
  // need to check the extrapolation worked?
  mop.getPosMomCov(posAtLayer, momAtLayer, covAtLayer);
  try {
    mop.extrapolateToCylinder(radius, origin, zaxis);
  } catch (...) {
    B2DEBUG(90, "Cylinder extrapolation failed! R:" << radius);
    // extrapolation failed
    return false;
  }
  B2DEBUG(171, "Old posAtLayer " << posAtLayer.X() << " " << posAtLayer.Y() << " " << posAtLayer.Z());
  mop.getPosMomCov(posAtLayer, momAtLayer, covAtLayer);
  B2DEBUG(171, "New posAtLayer " << posAtLayer.X() << " " << posAtLayer.Y() << " " << posAtLayer.Z());
  return true;
}

bool CDCToVXDExtrapolatorModule::extrapolateToPXDLayer(genfit::Track* track, int searchLayer, StoreArray<PXDCluster>& clusters)
{
  bool found = false;
  B2DEBUG(95, "<----> PXD extrapolateToLayer " << searchLayer << " <---->");

  static float layerRadii[7] = {0, 1.421, 2.179, 3.799, 8.0, 10.4, 13.51};

  // assumes the track is ordered correctly
  TVector3 posAtLayer, momAtLayer;
  TMatrixDSym covAtLayer(6);
  if (!extrapolateToCylinder(layerRadii[searchLayer], track, posAtLayer, momAtLayer, covAtLayer)) {
    return false;
  }

  int besti = -1; float bestDist = 1e10;
  float zSigma = sqrt(covAtLayer[2][2]);
  float xySigma = errDeltaXYDist(posAtLayer, covAtLayer);
  B2DEBUG(60, "Extrapolating to pixel layer: Pos:"
          << posAtLayer.X() << "," << posAtLayer.Y() << "," << posAtLayer.Z()
          << "Mom:" << momAtLayer.X() << "," << momAtLayer.Y() << "," << momAtLayer.Z());
  std::map<VxdID, TVector3> sensor2pos;
  std::map<VxdID, double> sensor2zSig;
  std::map<VxdID, double> sensor2xySig;
  for (int i = 0; i < clusters.getEntries(); ++i) {
    TVector3 oldPosition = posAtLayer;

    const auto& cluster = clusters[i];
    if (cluster->getSensorID().getLayerNumber() != searchLayer) continue;
    auto aSensorInfo = VXD::GeoCache::getInstance().getSensorInfo(cluster->getSensorID());
    TVector3 pos = getGlobalPosition(cluster->getU(), cluster->getV(), aSensorInfo);
    // check within a layer width/length
    double sensorWidth = aSensorInfo.getWidth() / 2.;
    TVector3 midpos = getGlobalPosition(0, cluster->getV(), aSensorInfo);
    TVector3 wpos = getGlobalPosition(sensorWidth, cluster->getV(), aSensorInfo);
    sensorWidth = fabs(midpos.DeltaPhi(wpos));

    float dz = pos.Z() - posAtLayer.Z();
    float dxy = deltaXYDist(pos, posAtLayer);
    B2DEBUG(101, "Good cluster. dZ:" << dz << " dXY:" << dxy << " swidth:" << sensorWidth << " lng:" << aSensorInfo.getLength());
    if ((fabs(dz) < aSensorInfo.getLength() / 2) && (dxy < sensorWidth)) {

      if (m_extrapolateToDetector) {
        if (sensor2pos.count(cluster->getSensorID()) > 0) {
          posAtLayer = sensor2pos[cluster->getSensorID()];
        } else {
          extrapolateToSensor(0, cluster, track, posAtLayer, momAtLayer, covAtLayer);
          sensor2pos[cluster->getSensorID()] = posAtLayer;
          sensor2zSig[cluster->getSensorID()] = sqrt(covAtLayer[2][2]);
          sensor2xySig[cluster->getSensorID()] = errDeltaXYDist(posAtLayer, covAtLayer);
        }
        zSigma = sensor2zSig[cluster->getSensorID()];
        xySigma = sensor2xySig[cluster->getSensorID()];
        dz = fabs(pos.Z() - posAtLayer.Z());
        dxy = deltaXYDist(pos, posAtLayer);
      }

      B2DEBUG(80, "Good cluster. dZ:" << dz << " dXY:" << dxy << " swidth:" << sensorWidth << " lng:" << aSensorInfo.getLength());
      double newDist = sqrt(dz * dz + dxy * dxy);
      if (m_searchSensorDimensions) {
        if ((besti < 0) || (newDist < bestDist)) {
          B2DEBUG(81, "New best");
          besti = i;
          bestDist = newDist;
        }
      } else {
        if ((dz < m_hitNSigmaPix * zSigma) && (dxy < m_hitNSigmaPix * xySigma)) {
          if ((besti < 0) || (newDist < bestDist)) {
            B2DEBUG(81, "New best");
            besti = i;
            bestDist = newDist;
          }
        }
      }
    }

    posAtLayer = oldPosition;
  }

  // Found a cluster
  if (besti >= 0) {
    found = true;
    auto cluster = clusters[besti];
    auto hit = new PXDRecoHit(cluster);
    track->insertMeasurement(hit, 0);
    B2DEBUG(60, "Inserted cluster.");
    if (m_saveInfo) {
      if (m_extrapolateToDetector) {
        posAtLayer = sensor2pos[cluster->getSensorID()];
        zSigma = sensor2zSig[cluster->getSensorID()];
        xySigma = sensor2xySig[cluster->getSensorID()];
      }
      auto aSensorInfo = VXD::GeoCache::getInstance().getSensorInfo(cluster->getSensorID());
      TVector3 pos = getGlobalPosition(cluster->getU(), cluster->getV(), aSensorInfo);
      stHitInfo.z = posAtLayer.Z() - pos.Z();
      stHitInfo.pull_z = fabs(stHitInfo.z) / zSigma;
      stHitInfo.rphi = deltaXYDist(posAtLayer, pos);
      stHitInfo.pull_rphi = stHitInfo.rphi / xySigma;
      TVector3 trkInlocal = aSensorInfo.pointToLocal(posAtLayer);
      stHitInfo.du = trkInlocal.X() - cluster->getU();
      stHitInfo.dv = trkInlocal.Y() - cluster->getV();
      stHitInfo.dw = trkInlocal.Z();
      stHitInfo.layer = searchLayer;
      stHitInfo.cone = false;

      bool truHitR = false;
      for (auto& clu : *pxdClustersFromMC) {

        if ((cluster->getSensorID() == clu.getSensorID()) && (clu.getU() == cluster->getU()) && (clu.getV() == cluster->getV()))
          truHitR  = true;
      }

      stHitInfo.isPxd = true;
      stHitInfo.truR = truHitR;
      stHitInfo.truZ = truHitR;
      HitInfo->Fill();
    }
  }

  B2DEBUG(95, "<----> End PXD extrapolateToLayer " << searchLayer << " <---->");

  return found;
}

bool CDCToVXDExtrapolatorModule::extrapolateToLayer(genfit::Track* track, int searchLayer, StoreArray<SVDCluster>& clusters)
{
  B2DEBUG(95, "<----> extrapolateToLayer " << searchLayer << " <---->");
  TVector3 zaxis(0, 0, 1);
  TVector3 mzaxis(0, 0, -1);
  TVector3 origin(0, 0, 0);
  // 1-indexed layer radii, numbers from xml files, should actually load them from these numbers?
  static float layerRadii[7] = {0, 1.421, 2.179, 3.799, 8.0, 10.4, 13.51};
  // similarly for the boundaries
  //static float boundaryZ[7] = {0, 1.421, 2.179, 3.799, 8.0, 10.4, 13.515};
  // for the layers with bent parts, at what z does the bent part start?
  static float bentZ[7] = {0, 0, 0, 0, 8.94, 17.51, 24.61};
  // and what is the opening angle of the "cone" created by the bent layers
  const float degToRad = 3.14159265 / 180.0;
  static float openingAngle[7] = {0, 0, 0, 0, 11.9f * degToRad, 16.0f * degToRad, 21.1f * degToRad};

  // assumes the track is ordered correctly
  TVector3 posAtLayer, momAtLayer;
  TMatrixDSym covAtLayer(6);
  if (!extrapolateToCylinder(layerRadii[searchLayer], track, posAtLayer, momAtLayer, covAtLayer)) {
    return false;
  }

  // check for slant layer
  bool isCone = false;
  if ((searchLayer > 3) && (posAtLayer.Z() > bentZ[searchLayer])) {
    // extrapolate to cone
    isCone = true;

    /// at bentZ[layer], the radius should match the cylinder, and given the opening angle, should know the beginning of the cone
    genfit::MeasuredStateOnPlane mop;
    auto coneVertex = TVector3(0, 0, bentZ[searchLayer] + layerRadii[searchLayer] / tan(openingAngle[searchLayer]));
    try {
      mop = track->getFittedState(0);
    } catch (...) {
      B2WARNING("Track without any fitted states after search!");
      return false;
    }
    try {
      B2DEBUG(190, "extrapolate to cone on layer " << searchLayer << " svd. On cylinder: " << posAtLayer.X() << " " << posAtLayer.Y() <<
              " " << posAtLayer.Z() << " " << " ConeV: " << coneVertex.Z() << " openingA: " << openingAngle[searchLayer]);
      mop.extrapolateToCone(openingAngle[searchLayer], coneVertex, mzaxis);
    } catch (...) {
      B2DEBUG(90, ">>>>> Conical extrapolation failed!");
    }
    mop.getPosMomCov(posAtLayer, momAtLayer, covAtLayer);
    B2DEBUG(190, ">>>>>>>>>>>> extrapolate to cone on layer " << searchLayer << " svd. On cone: " << posAtLayer.X() << " " <<
            posAtLayer.Y() << " " << posAtLayer.Z());
  }

  B2DEBUG(190, "end of extrapolation");
  B2DEBUG(190, "extrapolated to layer " << searchLayer << " svd");
  B2DEBUG(101, "   ------------<> Extrapolation result: Position " << posAtLayer.x() << " " << posAtLayer.y() << " " << posAtLayer.z()
          <<
          " Momentum " << momAtLayer.x() << " " << momAtLayer.y() << " " << momAtLayer.z() << " Cov_ii " << covAtLayer[0][0] << " " <<
          covAtLayer[1][1]
          << " " << covAtLayer[2][2] << " " << covAtLayer[3][3] << " " << covAtLayer[4][4] << " " << covAtLayer[5][5]);

  int bestv = -1, bestu = -1; float bestDist = 1e10;

  float zSigma = sqrt(covAtLayer[2][2]);
  double r = sqrt(pow(posAtLayer.X(), 2) + pow(posAtLayer.Y(), 2));
  B2DEBUG(190, "   --->Track Position r: " << r << " " << posAtLayer.Perp() << " z:" << posAtLayer.Z() <<
          " zsigma:" << sqrt(covAtLayer[2][2]) << " VXX:" << covAtLayer[0][0] << " sXX:" << sqrt(covAtLayer[0][0]) << " VYY:" <<
          covAtLayer[1][1] << " sYY:" << sqrt(covAtLayer[1][1]) << " VXY:" << covAtLayer[0][1]);
  // try to find a v cluster

  std::map<VxdID, TVector3> sensor2pos;
  std::map<VxdID, double> sensor2zsigma;
  for (int i = 0; i < clusters.getEntries(); ++i) {
    const auto& cluster = clusters[i];

    if (cluster->isUCluster()) continue;
    if (cluster->getSensorID().getLayerNumber() != searchLayer) continue;

    auto aSensorInfo = VXD::GeoCache::getInstance().getSensorInfo(cluster->getSensorID());
    TVector3 pos = getGlobalPosition(0, cluster->getPosition(), aSensorInfo);
    double sensorWidth = aSensorInfo.getWidth() / 2.;
    TVector3 midpos = getGlobalPosition(0, cluster->getPosition(), aSensorInfo);
    TVector3 wpos = getGlobalPosition(sensorWidth, cluster->getPosition(), aSensorInfo);
    sensorWidth = fabs(midpos.DeltaPhi(wpos));

    B2DEBUG(221, "Looking for V cluster dz: " << fabs(pos.Z() - posAtLayer.Z()) << " dphi:" << deltaXYDist(posAtLayer,
            pos) << " dz:" << fabs(
              pos.Z() - posAtLayer.Z()));
    if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 221, PACKAGENAME()) == true) {
      pos.Print();
    }

    double absdz = fabs(pos.Z() - posAtLayer.Z());
    double dxy = fabs(deltaXYDist(posAtLayer, pos));
    if ((absdz < aSensorInfo.getLength()) && (dxy < sensorWidth)) {
      TVector3 oldPosition(posAtLayer);
      if (m_extrapolateToDetector) {
        if (sensor2pos.count(cluster->getSensorID()) > 0) {
          posAtLayer = sensor2pos[cluster->getSensorID()];
        } else {
          if (!extrapolateToSensor(cluster, 0, track, posAtLayer, momAtLayer, covAtLayer)) {
            continue;
          }
          sensor2pos[cluster->getSensorID()] = posAtLayer;
          sensor2zsigma[cluster->getSensorID()] = sqrt(covAtLayer[2][2]);
        } // sensor2pos.count
        // fix the values to that on the sensor
        absdz = fabs(pos.Z() - posAtLayer.Z());
        zSigma = sensor2zsigma[cluster->getSensorID()];
      } // m_extrapolateToDetector

      if ((bestv >= 0) && (absdz > bestDist)) {
        B2DEBUG(171, "Not Better");
        posAtLayer = oldPosition;
        continue;
      }

      if (m_searchSensorDimensions) {
        if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 220, PACKAGENAME()) == true) {
          pos.Print();
        }
        bestv = i;
        bestDist = absdz;
      } else {
        if (absdz < m_hitNSigmaZ * zSigma) {
          bestv = i;
          bestDist = absdz;
        }
      }
      posAtLayer = oldPosition;
    }
  }
  if (bestv >= 0) {
    B2DEBUG(190, "FOUND A V CLUSTER");
    const auto vCluster = clusters[bestv];
    auto aSensorInfo = VXD::GeoCache::getInstance().getSensorInfo(vCluster->getSensorID());
    TVector3 svdPos = getGlobalPosition(0, vCluster->getPosition(), aSensorInfo);
    TVector3 svdPos1Sig = getGlobalPosition(0, vCluster->getPosition() + vCluster->getPositionSigma(), aSensorInfo);
    TVector3 svdErr = svdPos1Sig - svdPos;
    B2DEBUG(190, "Sensor Id: " << vCluster->getSensorID() << " PosOnSensor: " << vCluster->getPosition() <<
            " PosSigma: " << vCluster->getPositionSigma() <<
            " GloablPos: " << svdPos.X() << " " << svdPos.Y() << " " << svdPos.Z() <<
            " GloablPos1Sig: " << svdPos1Sig.X() << " " << svdPos1Sig.Y() << " " << svdPos1Sig.Z() <<
            " Err: " << svdErr.X() << " " << svdErr.Y() << " " << svdErr.Z() <<
            " isU: " << vCluster->isUCluster());

    // extrpolate to the correct detector
    if (m_extrapolateToDetector) {
      extrapolateToSensor(vCluster, 0, track, posAtLayer, momAtLayer, covAtLayer);
    }


    // find a matching u cluster
    auto vSensorInfo = VXD::GeoCache::getInstance().getSensorInfo(vCluster->getSensorID());
    bestDist = -1;
    double sensorWidth = aSensorInfo.getWidth() / 2.;
    TVector3 midpos = getGlobalPosition(0, vCluster->getPosition(), aSensorInfo);
    TVector3 wpos = getGlobalPosition(sensorWidth, vCluster->getPosition(), aSensorInfo);
    sensorWidth = fabs(midpos.DeltaPhi(wpos));
    for (int i = 0; i < clusters.getEntries(); ++i) {
      const auto& cluster = clusters[i];
      if (!(cluster->getSensorID() == vCluster->getSensorID())) continue;
      if (!cluster->isUCluster()) continue;
      // only hits on the same sensor
      TVector3 svdPos = getGlobalPosition(cluster->getPosition(), vCluster->getPosition(), vSensorInfo);
      TVector3 svdPos1Sig = getGlobalPosition(cluster->getPosition() + cluster->getPositionSigma(), vCluster->getPosition(), vSensorInfo);
      TVector3 svdErr = svdPos1Sig - svdPos;
      double dxy = deltaXYDist(posAtLayer, svdPos);
      if ((bestu < 0) || (dxy < bestDist)) {
        B2DEBUG(201, "Sensor Dphi " << sensorWidth << " midposphi: " << midpos.Phi() << " wposphi: " << wpos.Phi());

        if (m_searchSensorDimensions) {
          B2DEBUG(220, "Looking for U cluster dphi: " << deltaXYDist(posAtLayer, svdPos));
          if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 220, PACKAGENAME()) == true) {
            svdPos.Print();
          }
          if (dxy < sensorWidth) {
            bestu = i;
            bestDist = dxy;
          }
        } else {
          if (dxy < m_hitNSigmaXY * errDeltaXYDistSVD(posAtLayer, covAtLayer, svdErr)) {
            bestu = i;
            bestDist = dxy;
          }
        }
        // bestu = i;
        // bestDist = deltaXYDist2(svdPos);
      }
    }
    if (bestu >= 0) {
      B2DEBUG(190, "FOUND A U CLUSTER");
      const auto uCluster = clusters[bestu];
      auto aSensorInfo = VXD::GeoCache::getInstance().getSensorInfo(uCluster->getSensorID());
      TVector3 svdPos = getGlobalPosition(uCluster->getPosition(), vCluster->getPosition(), aSensorInfo);
      TVector3 svdPos1Sig = getGlobalPosition(uCluster->getPosition() + uCluster->getPositionSigma(), vCluster->getPosition(),
                                              aSensorInfo);
      TVector3 svdErr = svdPos1Sig - svdPos;
      TVector3 trkInlocal = aSensorInfo.pointToLocal(posAtLayer);
      B2DEBUG(190, "Sensor Id: " << uCluster->getSensorID() << " PosOnSensor: " << uCluster->getPosition() <<
              " PosSigma: " << uCluster->getPositionSigma() <<
              " GloablPos: " << svdPos.X() << " " << svdPos.Y() << " " << svdPos.Z() <<
              " GloablPos1Sig: " << svdPos1Sig.X() << " " << svdPos1Sig.Y() << " " << svdPos1Sig.Z() <<
              " isU: " << uCluster->isUCluster() << " PosAtLayerLocal:" << trkInlocal.X() << "," << trkInlocal.Y() << "," << trkInlocal.Z());
      // this is how the error is found in the SpacePointCreator
      B2Vector3<double> spPositionError = aSensorInfo.vectorToGlobal(TVector3(uCluster->getPositionSigma() * uCluster->getPositionSigma(),
                                          vCluster->getPositionSigma() * vCluster->getPositionSigma(),
                                          0));
      spPositionError.Sqrt();
      TVector3 spErr = spPositionError.GetTVector3();

      TVector3 orr = TVector3(0, 0, 0);
      B2DEBUG(190, "Errors: r:" << r << " dr:" << deltaXYDist(posAtLayer, svdPos) << " d(dr):" <<
              errDeltaXYDist(posAtLayer, covAtLayer) << " d(dr)svd:" << errDeltaXYDistSVD(posAtLayer, covAtLayer, svdErr) <<
              " ddrsvdsp:" << errDeltaXYDistSVD(posAtLayer, covAtLayer, spErr) << " d(dr) from 0:" <<
              errDeltaXYDist(posAtLayer, covAtLayer) << " d(dr) from 0[corr]:" << errDeltaXYDistSVD(posAtLayer, covAtLayer, spErr));
      TVector3 pos2d = getGlobalPosition(uCluster->getPosition(), vCluster->getPosition(), aSensorInfo);
      B2DEBUG(90, "<-----> Final CLUSTER Position: " << pos2d.x() << " " << pos2d.y() << " " << pos2d.z());

      // add hits to track
      auto vHit = new SVDRecoHit(vCluster);
      auto uHit = new SVDRecoHit(uCluster);
      track->insertMeasurement(vHit, 0);
      track->insertMeasurement(uHit, 0);

      if (m_stepwiseKalman) {
        // Do a partial refit step
        bool fitSuccess = false;
        // following the GenFitter.cc code
        auto fitter = new genfit::KalmanFitter();
        fitter->setMinIterations(3);
        fitter->setMaxIterations(10);
        fitter->setMultipleMeasurementHandling(genfit::unweightedClosestToPredictionWire);
        fitter->setMaxFailedHits(5);
        try {
          try {
            track->checkConsistency();
          } catch (genfit::Exception& e) {
            B2DEBUG(50, e.getExcString());
            B2DEBUG(50, "Inconsistent track found, attempting to sort!");
            bool sorted = track->sort();
            if (!sorted) {
              B2DEBUG(50, "Track points NOT SORTED! Still inconsistent, I can't be held responsible for assertion failure!");
            } else {
              B2DEBUG(50, "Track points SORTED!! Hopefully this works now!");
            }
          }
          fitter->processTrackPartially(track, track->getCardinalRep(), 1, 0);

          genfit::FitStatus* fs = 0;
          genfit::KalmanFitStatus* kfs = 0;
          fitSuccess = track->hasFitStatus();
          if (fitSuccess) {
            fs = track->getFitStatus();
            fitSuccess = fitSuccess && fs->isFitted();
            fitSuccess = fitSuccess && fs->isFitConverged();
            kfs = dynamic_cast<genfit::KalmanFitStatus*>(fs);
            fitSuccess = fitSuccess && kfs;
            double fchi2(0), bchi2(0), fndf(0), bndf(0);
            auto up0 = track->getPointWithMeasurementAndFitterInfo(0, track->getCardinalRep())->getKalmanFitterInfo(0)->getUpdate(-1);
            auto up1 = track->getPointWithMeasurementAndFitterInfo(1, track->getCardinalRep())->getKalmanFitterInfo(0)->getUpdate(-1);
            // fitter->getChiSquNdf(track, track->getCardinalRep(), bchi2, fchi2, bndf, fndf);
            B2DEBUG(100, "<------> Track partial fit. isFitted: " << fs->isFitted() << " isFitConverged:" << fs->isFitConverged() <<
                    " isFitConverged(false):"
                    << fs->isFitConverged(false) << " nFailedPoints:" << fs->getNFailedPoints() << " numPointsWithMeasurement (from track): " <<
                    track->getNumPointsWithMeasurement() << " chi2 " << track->getFitStatus(track->getCardinalRep())->getChi2() << " ndof " <<
                    track->getFitStatus(track->getCardinalRep())->getNdf() <<
                    " chi2 up0 " << up0->getChiSquareIncrement() << " chi2 up1 " << up1->getChiSquareIncrement() <<
                    // " chi2: " << fitter->getChiSqu(track, track->getCardinalRep()) <<
                    // << " Ndof: " << fitter->getNdf(track, track->getCardinalRep()) << " redchi2: " << fitter->getRedChiSqu(track, track->getCardinalRep()) <<
                    " bchi2 " << bchi2 << " bndf " << bndf << " fchi2 " << fchi2 << " fndf " << fndf);
            trkInfo.pval = fs->getPVal();
          } else {
            B2WARNING("Bad Fit in CDCToVXDExtrapolatorModule");
          }
        } catch (...) {
          B2WARNING("Track fitting has failed.");
        }
        delete fitter;

        TMatrixDSym cov(6, 6);
        TVector3 pos, mom;
        track->getFittedState().getPosMomCov(pos, mom, cov);
        B2DEBUG(101, "<---------> Track partial fit result: Position " << pos.x() << " " << pos.y() << " " << pos.z() <<
                " Momentum " << mom.x() << " " << mom.y() << " " << mom.z() << " Cov_ii " << cov[0][0] << " " << cov[1][1]
                << " " << cov[2][2] << " " << cov[3][3] << " " << cov[4][4] << " " << cov[5][5]);
      }

      if (m_saveInfo) {
        // fill hit info
        stHitInfo.z = posAtLayer.Z() - pos2d.Z();
        stHitInfo.pull_z = fabs(posAtLayer.Z() - pos2d.Z()) / sqrt(zSigma * zSigma + spErr.Z() * spErr.Z());
        stHitInfo.rphi = deltaXYDist(posAtLayer, pos2d);
        stHitInfo.pull_rphi = stHitInfo.rphi / errDeltaXYDistSVD(posAtLayer, covAtLayer, svdErr);
        stHitInfo.du = trkInlocal.X() - uCluster->getPosition();
        stHitInfo.dv = trkInlocal.Y() - vCluster->getPosition();
        stHitInfo.dw = trkInlocal.Z();
        stHitInfo.layer = searchLayer;
        stHitInfo.cone = isCone ? 1 : 0;

        bool truHitR = false, truHitZ = false;
        for (auto& clu : *svdClustersFromMC) {
          if ((uCluster->getSensorID() == clu.getSensorID()) && (clu.isUCluster() == uCluster->isUCluster())
              && (clu.getPosition() == uCluster->getPosition()))
            truHitR  = true;
          if ((vCluster->getSensorID() == clu.getSensorID()) && (clu.isUCluster() == vCluster->isUCluster())
              && (clu.getPosition() == vCluster->getPosition()))
            truHitZ  = true;
        }

        stHitInfo.isPxd = false;
        stHitInfo.truR = truHitR;
        stHitInfo.truZ = truHitZ;
        HitInfo->Fill();
      }
    } else {
      B2DEBUG(190, "NO MATCHING U CLUSTER FOUND FOR TRACK");
    }
  } else {
    B2DEBUG(190, "NO MATCHING V CLUSTER FOUND FOR TRACK");
  }
  B2DEBUG(95, "<----> End extrapolateToLayer " << searchLayer << " <---->");
  return (bestv >= 0) && (bestu >= 0);
}

bool CDCToVXDExtrapolatorModule::extrapolateThroughSvd(genfit::Track* track, StoreArray<SVDCluster>& svdClusters,
                                                       StoreArray<PXDCluster>& pxdClusters)
{
  // extrpaolate through the whole SVD
  bool foundSvd = false, foundPxd = false;
  bool found5 = false, found4 = false, found3 = false, found2 = false, found1 = false;
  bool found6 = extrapolateToLayer(track, 6, svdClusters);
  foundSvd = foundSvd || found6;
  if (found6 || m_allLayers) {
    found5 = extrapolateToLayer(track, 5, svdClusters);
    foundSvd = foundSvd || found5;
    if (found5 || m_allLayers) {
      found4 = extrapolateToLayer(track, 4, svdClusters);
      foundSvd = foundSvd || found4;
      if (found4 || m_allLayers) {
        found3 = extrapolateToLayer(track, 3, svdClusters);
        foundSvd = foundSvd || found3;
        /// Require a hit to be found in the pxd to continue the extrapolation regardless of settings
        if (foundSvd && m_extrapolateToPxd) {
          found2 = extrapolateToPXDLayer(track, 2, pxdClusters);
          if (found2) {
            found1 = extrapolateToPXDLayer(track, 1, pxdClusters);
          }
          foundPxd = found2 || found1;
        }
      }
    }
  }
  if (foundSvd) {
    nTracksWithAddedHits++;
  }
  if (foundPxd) {
    nTracksWithAddedPxdHits++;
  }
  return foundSvd;
}


bool CDCToVXDExtrapolatorModule::refitTrack(genfit::Track* track)
{
  bool fitSuccess = false;
  // following the GenFitter.cc code
  genfit::AbsKalmanFitter* fitter = 0;
  if (m_useKalman) {
    fitter = new genfit::KalmanFitterRefTrack();
    fitter->setMinIterations(3);
    fitter->setMaxIterations(10);
    fitter->setMultipleMeasurementHandling(genfit::unweightedClosestToPredictionWire);
    fitter->setMaxFailedHits(5);
  } else {
    fitter = new genfit::DAF(true);
    ((genfit::DAF*) fitter)->setProbCut(0.001);
    fitter->setMaxFailedHits(5);
  }
  try {
    try {
      track->checkConsistency();
    } catch (genfit::Exception& e) {
      B2DEBUG(50, e.getExcString());
      B2DEBUG(50, "Inconsistent track found, attempting to sort!");
      bool sorted = track->sort();
      if (!sorted) {
        B2DEBUG(50, "Track points NOT SORTED! Still inconsistent, I can't be held responsible for assertion failure!");
      } else {
        B2DEBUG(50, "Track points SORTED!! Hopefully this works now!");
      }
    }

    fitter->processTrack(track);

    genfit::FitStatus* fs = 0;
    genfit::KalmanFitStatus* kfs = 0;
    fitSuccess = track->hasFitStatus();
    if (fitSuccess) {
      fs = track->getFitStatus();
      fitSuccess = fitSuccess && fs->isFitted();
      fitSuccess = fitSuccess && fs->isFitConverged();
      kfs = dynamic_cast<genfit::KalmanFitStatus*>(fs);
      fitSuccess = fitSuccess && kfs;
      B2DEBUG(100, "> Track fit. isFitted: " << fs->isFitted() << " isFitConverged:" << fs->isFitConverged() << " isFitConverged(false):"
              << fs->isFitConverged(false) << " nFailedPoints:" << fs->getNFailedPoints() << " numPointsWithMeasurement (from track): " <<
              track->getNumPointsWithMeasurement());
      trkInfo.pval = fs->getPVal();
    } else {
      B2WARNING("Bad Fit in CDCToVXDExtrapolatorModule");
    }
  } catch (...) {
    B2WARNING("Track fitting has failed.");
  }
  delete fitter;
  return fitSuccess;
}

void CDCToVXDExtrapolatorModule::initialize()
{
  B2DEBUG(80, "Entered CDCToVXDExtrapolatorModule::initialize()");
  // Input
  StoreArray<MCParticle>::required(m_mcParticlesColName);
  StoreArray<genfit::Track>::required(m_GFTrackColName);
  StoreArray<CDCHit>::required();
  StoreArray<SVDCluster>::required();
  StoreArray<PXDTrueHit>::required();
  StoreArray<MCParticle> mcParticles;
  mcParticles.isOptional();
  // output
  StoreArray<genfit::Track>::registerPersistent(m_OutGFTrackColName);
  StoreArray<genfit::TrackCand>::registerPersistent(m_OutGFTrackCandColName);

  // register relations
  StoreArray<genfit::Track> gftracks(m_OutGFTrackColName);
  StoreArray<genfit::TrackCand> gfTrackCands(m_OutGFTrackCandColName);
  gfTrackCands.registerRelationTo(gftracks);
  gftracks.registerRelationTo(gfTrackCands);
  gftracks.registerRelationTo(mcParticles);

  // setup magnetic field
  if (!genfit::FieldManager::getInstance()->isInitialized()) {
    B2WARNING("Magnetic field not set up, doing this myself.");

    // pass the magnetic field to genfit
    genfit::FieldManager::getInstance()->init(new GFGeant4Field());
    genfit::FieldManager::getInstance()->useCache();
  }


  m_saveInfo = false;
  if (m_rootOutputFilename != std::string("")) {
    m_outputFile = new TFile(m_rootOutputFilename.c_str(), "recreate");

    m_saveInfo = true;

    HitInfo = new TTree("HitInfo", "HitInfo");
    HitInfo->Branch("stHitInfo", &stHitInfo,
                    "z/F:rphi/F:pull_z/F:pull_rphi/F:du/F:dv/F:dw/F:layer/I:cone/I:truR/I:truZ/I:isPxd/O");
    TrackInfo = new TTree("TrackInfo", "TrackInfo");
    TrackInfo->Branch("trkInfo", &trkInfo,
                      "StartHitIdx/i:EndHitIdx/i:pt/F:th/F:d0/F:pval/F:truPxd/i:truSvd/i:nRec/i:cdconly/O:refit/O");
  }
}

void CDCToVXDExtrapolatorModule::storeTrack(genfit::Track& crnt, StoreArray<genfit::Track>& outGfTracks,
                                            StoreArray<genfit::TrackCand>& outGfTrackCands,
                                            RelationArray& gfTrackCandidatesTogfTracks,
                                            RelationArray& gfTracksToMCPart)
{
  // store gftrack
  genfit::Track* extrapolatedTrack = outGfTracks.appendNew(crnt);

  // store the TrackCand
  genfit::TrackCand* theCand = extrapolatedTrack->constructTrackCand();
  //genfit::TrackCand* extrCand =
  outGfTrackCands.appendNew(*theCand);
  delete theCand;
  gfTrackCandidatesTogfTracks.add(nOutTracks, nOutTracks);

  gfTracksToMCPart.add(nOutTracks, crnt.getMcTrackId());

  nOutTracks++;
  nStoredTracks++;
}

void getHits(genfit::Track* track, uint& nPxd, uint& nSvd2, uint& nSvd, uint& nCdc, uint& nUnk)
{
  nPxd = 0, nSvd2 = 0, nSvd = 0, nCdc = 0, nUnk = 0;
  for (auto pt : track->getPointsWithMeasurement()) {
    for (auto absMeas : pt->getRawMeasurements())
      if (dynamic_cast<PXDRecoHit*>(absMeas))
        ++nPxd;
      else if (dynamic_cast<SVDRecoHit2D*>(absMeas))
        ++nSvd2;
      else if (dynamic_cast<SVDRecoHit*>(absMeas))
        ++nSvd;
      else if (dynamic_cast<CDCRecoHit*>(absMeas))
        ++nCdc;
      else
        ++nUnk;
  }
  B2DEBUG(80, "-- Track Hits: nMeas: " << track->getPointsWithMeasurement().size() << " nAll: " << track->getPoints().size()  <<
          " -- nPxd " << nPxd << " nSvd2 " << nSvd2 << " nSvd " << nSvd << " nCdc " << nCdc << " nUnk " << nUnk);
}

void CDCToVXDExtrapolatorModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  int eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(80, "<--> CDCToVXDExtrapolatorModule::event() : " << eventCounter << " <-->");
  StoreArray<genfit::Track> gfTracks(m_GFTrackColName);
  StoreArray<MCParticle> mcParticles(m_mcParticlesColName);
  int nTracks = gfTracks.getEntries();
  StoreArray<genfit::Track> outGfTracks(m_OutGFTrackColName);
  StoreArray<genfit::TrackCand> outGfTrackCands(m_OutGFTrackCandColName);
  RelationArray gfTrackCandidatesTogfTracks(outGfTrackCands, outGfTracks);
  RelationArray gfTracksToMCPart(outGfTracks, mcParticles);

  StoreArray<PXDCluster> pxdClusters;
  StoreArray<SVDCluster> svdClusters;
  StoreArray<CDCHit> cdcHits;
  StoreArray<SVDRecoHit> svdRecoHits;

  B2DEBUG(80, "Number of tracks in the genfit::Track StoreArray: " <<  nTracks);
  uint nPxd = 0, nSvd2 = 0, nSvd = 0, nCdc = 0, nUnk = 0;
  nOutTracks = 0; // number of tracks in our output array
  for (genfit::Track& crnt : gfTracks) {
    B2DEBUG(110, "<---> Processing next track " << nOutTracks << " <--->");

    if (m_saveInfo) {
      trkInfo.refit = false;
      trkInfo.cdconly = false;
      trkInfo.StartHitIdx = HitInfo->GetEntries();
      trkInfo.truSvd = 0;
      trkInfo.truPxd = 0;
      trkInfo.nRec = 0;
    }

    // should only be true if the track gets output
    bool isSaved = false;
    nTotalTracks++;
    getHits(&crnt, nPxd, nSvd2, nSvd, nCdc, nUnk);
    bool isCdcRecoOnly = false;
    if ((nPxd == 0) && (nSvd2 == 0) && (nSvd == 0)) {
      nCdcRecoOnlyTracks++;
      isCdcRecoOnly = true;
      trkInfo.cdconly = true;
    }

    RelationVector<MCParticle> MCParticles_fromTrack = DataStore::getRelationsFromObj<MCParticle>(&crnt);
    B2DEBUG(110, "--- Number of related MC particles: " << MCParticles_fromTrack.size());

    genfit::Track* mcTrk = 0;
    genfit::Track* extrapolatedTrack = 0;
    try {
      if (m_refitMcHits) {
        mcTrk = new genfit::Track(crnt);
      }
      float mcpt = -1;
      float mcd0 = -1;
      float mcth = -1;
      bool recoverable = false;
      int nCdcMcHits = 0;
      int nSvdMcClusters = 0;
      int nPxdMcClusters = 0;
      if (MCParticles_fromTrack.size() == 1) {
        mcpt = MCParticles_fromTrack[0]->get4Vector().Pt();
        mcth = MCParticles_fromTrack[0]->get4Vector().Theta() * 180 / 3.14159265;
        mcd0 = MCParticles_fromTrack[0]->getProductionVertex().Perp();
        trkInfo.pt = mcpt; trkInfo.th = mcth; trkInfo.d0 = mcd0;

        if (cdcHitsFromMC) delete cdcHitsFromMC;
        if (svdClustersFromMC) delete svdClustersFromMC;
        if (pxdClustersFromMC) delete pxdClustersFromMC;
        cdcHitsFromMC = new RelationVector<CDCHit>(DataStore::getRelationsFromObj<CDCHit>(MCParticles_fromTrack[0]));
        svdClustersFromMC = new RelationVector<SVDCluster>(DataStore::getRelationsToObj<SVDCluster>(MCParticles_fromTrack[0]));
        pxdClustersFromMC = new RelationVector<PXDCluster>(DataStore::getRelationsToObj<PXDCluster>(MCParticles_fromTrack[0]));

        nCdcMcHits = cdcHitsFromMC->size();
        nSvdMcClusters = svdClustersFromMC->size();
        nPxdMcClusters = pxdClustersFromMC->size();

        trkInfo.truSvd = svdClustersFromMC->size();
        trkInfo.truPxd = pxdClustersFromMC->size();

        B2DEBUG(90, "--- MC Assoc.: nCdc " << nCdcMcHits << " nSvd " << nSvdMcClusters << " nPxd " << nPxdMcClusters);
        if (isCdcRecoOnly && ((nSvdMcClusters > 0) || (nPxdMcClusters > 0))) {
          nCdcRecoOnlyTracksWithVxdMcHits++;
          recoverable = true;
          B2DEBUG(70, "<><><> Found MC VXD Hits Associated to CDC-Only Track, Refitting with MC Hits <><><>");

          int insHit = 0; // insertMeasurement place the cluster before the given index, need to build up

          if (m_refitMcHits) {
            for (auto& pxdCluster : *pxdClustersFromMC) {
              auto hit = new PXDRecoHit(&pxdCluster);
              mcTrk->insertMeasurement(hit, insHit++);
            }

            for (auto& svdCluster : *svdClustersFromMC) {
              auto hit = new SVDRecoHit(&svdCluster);
              mcTrk->insertMeasurement(hit, insHit++);
            }
          }
        } else {
          if (mcTrk)
            delete mcTrk;
          mcTrk = 0;
        }
      } else {
        if (mcTrk)
          delete mcTrk;
        mcTrk = 0;
        B2DEBUG(80, "--- No (or too many) MC particles associated to track Nassoc:" << MCParticles_fromTrack.size());
      }

      if (isCdcRecoOnly) {
        extrapolatedTrack = new genfit::Track(crnt);

        try {
          if (!extrapolateThroughSvd(extrapolatedTrack, svdClusters, pxdClusters)) {
            delete extrapolatedTrack;
            extrapolatedTrack = 0;
          } else {
            B2DEBUG(70, "<><> Added Extrapolated SVD Hits To Track <><>");
            if (m_saveInfo) {
              if (recoverable) { // real hits
                int nDiff = extrapolatedTrack->getNumPointsWithMeasurement() - crnt.getNumPointsWithMeasurement();
                trkInfo.nRec = nDiff;
                B2DEBUG(70, "nDiff:" << nDiff << " nSvdMc:" << nSvdMcClusters << " nPxdMc:" << nPxdMcClusters << " nCdcMc:" << nCdcMcHits <<
                        " nCrnt:" << crnt.getNumPointsWithMeasurement());
              }
            }
          }
        } catch (...) {
          B2DEBUG(150, "><><><>< Problem extrapolating track! ><><><><");
          delete extrapolatedTrack;
          extrapolatedTrack = 0;
        }
      }


      // now refit the tracks
      bool refitExtrap = false;
      bool refitMC = false;
      if (extrapolatedTrack) {
        try {
          refitExtrap = refitTrack(extrapolatedTrack);
        } catch (...) {
          B2DEBUG(50, "><><> Problem refitting track! <><><");
          refitExtrap = false;
        }
        if (!refitExtrap) {
          B2DEBUG(110, "Bad refit of extrap");
        }
      }
      if (mcTrk) {
        refitMC = refitTrack(mcTrk);
        if (!refitMC) {
          B2DEBUG(110, "Bad refit of MC");
        }
      }

      if (refitExtrap) {
        storeTrack(*extrapolatedTrack, outGfTracks, outGfTrackCands, gfTrackCandidatesTogfTracks, gfTracksToMCPart);
        isSaved = true;
        trkInfo.refit = true;
      } else {
        delete extrapolatedTrack;
        extrapolatedTrack = 0;
      }

      // compare them
      if (refitMC && refitExtrap) {
        if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 110, PACKAGENAME()) == true) {
          genfit::MeasuredStateOnPlane cdcNewMop = extrapolatedTrack->getFittedState();
          genfit::MeasuredStateOnPlane cdcMcMop = mcTrk->getFittedState();
          genfit::MeasuredStateOnPlane cdcOldMop = crnt.getFittedState();
          TVector3 oldPos, newPos, oldDir, newDir, mcPos, mcDir;
          TMatrixDSym oldCov(6), newCov(6), mcCov(6);
          TVector3 prod = MCParticles_fromTrack[0]->getProductionVertex();
          try {
            cdcNewMop.extrapolateToPoint(prod);
            cdcNewMop.getPosMomCov(newPos, newDir, newCov);
            cdcMcMop.extrapolateToPoint(prod);
            cdcMcMop.getPosMomCov(mcPos, mcDir, mcCov);
            cdcOldMop.extrapolateToPoint(prod);
            cdcOldMop.getPosMomCov(oldPos, oldDir, oldCov);
          } catch (...) {
            B2DEBUG(110, "Warning: Couldn't extrapolate properly. Results below are non-sensical");
          }
          B2DEBUG(110, "True Prod Position");
          prod.Print();
          B2DEBUG(110, "New Position");
          newPos.Print();
          B2DEBUG(110, "MC Position");
          mcPos.Print();
          B2DEBUG(110, "Old Position");
          oldPos.Print();
          B2DEBUG(110, "------");
          B2DEBUG(110, "True Momentum");
          MCParticles_fromTrack[0]->getMomentum().Print();
          B2DEBUG(110, "MC momentum");
          mcDir.Print();
          B2DEBUG(110, "New momentum");
          newDir.Print();
          B2DEBUG(110, "Old momentum");
          oldDir.Print();
          //B2DEBUG(110, "New covariance");
          //newCov.Print();
          //B2DEBUG(110, "Old covariance");
          //oldCov.Print();
        }
      }
      if (mcTrk) {
        delete mcTrk;
        mcTrk = 0;
      }
      // generic errors from track fit that were uncaught
    } catch (genfit::Exception& e) {
      B2WARNING("genfit::Exception caught in CDCToVXDExtrapolatorModule : " << e.getExcString());
    } catch (...) {
      B2WARNING("Uncaught, unknown exception in CDCToVXDExtrapolatorModule");
    }


    // dump out non cdc-only tracks if we are required to save all the tracks
    if (m_saveAllTracks) {
      try {
        if (!isSaved) {
          storeTrack(crnt, outGfTracks, outGfTrackCands, gfTrackCandidatesTogfTracks, gfTracksToMCPart);
        }
      } catch (genfit::Exception& e) {
        B2WARNING("genfit::Exception caught while trying to dump a default track in CDCToVXDExtrapolatorModule : " << e.getExcString());
      } catch (...) {
        B2WARNING("Unknown exception just trying to dump out a default track. Very strange.");
      }
    }
    if (m_saveInfo) {
      trkInfo.EndHitIdx = HitInfo->GetEntries();
      TrackInfo->Fill();
    }
    B2DEBUG(110, "<---> Finished track " << nOutTracks << " <--->");
  }
  B2DEBUG(110, "<--> Finished Event " << eventCounter << " <-->");
}

void CDCToVXDExtrapolatorModule::terminate()
{
  B2INFO("Terminated CDCToVXDExtrapolatorModule");
  B2INFO("nTotalTracks: " << nTotalTracks);
  B2INFO("nCdcRecoOnlyTracks: " << nCdcRecoOnlyTracks);
  B2INFO("nCdcRecoOnlyTracksWithVxdMcHits: " << nCdcRecoOnlyTracksWithVxdMcHits);
  B2INFO("nTracksWithAddedHits: " << nTracksWithAddedHits);
  B2INFO("nTracksWithAddedPxdHits: " << nTracksWithAddedPxdHits);
  B2INFO("nStoredTracks: " << nStoredTracks);

  if (m_saveInfo) {
    m_outputFile->cd();

    HitInfo->Write();
    TrackInfo->Write();

    m_outputFile->Close();
  }
}

