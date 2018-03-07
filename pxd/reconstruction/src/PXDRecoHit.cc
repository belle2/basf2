/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Martin Ritter, Moritz Nadler            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <pxd/reconstruction/HitCorrector.h>
#include <pxd/reconstruction/PXDClusterPositionEstimator.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/SensorPlane.h>
#include <vxd/geometry/GeoCache.h>

#include <genfit/DetPlane.h>
#include <TVector3.h>
#include <TRandom.h>

using namespace std;
using namespace Belle2;

PXDRecoHit::PXDRecoHit():
  genfit::PlanarMeasurement(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(0), m_cluster(0),
  m_energyDep(0)//, m_energyDepError(0)
{}

PXDRecoHit::PXDRecoHit(const PXDTrueHit* hit, const genfit::TrackCandHit*, float sigmaU, float sigmaV):
  genfit::PlanarMeasurement(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(hit), m_cluster(0),
  m_energyDep(0)//, m_energyDepError(0)
{
  if (!gRandom) B2FATAL("gRandom not initialized, please set up gRandom first");

  // Set the sensor UID
  m_sensorID = hit->getSensorID();

  //If no error is given, estimate the error by dividing the pixel size by sqrt(12)
  if (sigmaU < 0 || sigmaV < 0) {
    const PXD::SensorInfo& geometry = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));
    sigmaU = geometry.getUPitch(hit->getV()) / sqrt(12);
    sigmaV = geometry.getVPitch(hit->getV()) / sqrt(12);
  }

  // Set positions
  rawHitCoords_(0) = gRandom->Gaus(hit->getU(), sigmaU);
  rawHitCoords_(1) = gRandom->Gaus(hit->getV(), sigmaV);
  // Set the error covariance matrix
  rawHitCov_(0, 0) = sigmaU * sigmaU;
  rawHitCov_(0, 1) = 0;
  rawHitCov_(1, 0) = 0;
  rawHitCov_(1, 1) = sigmaV * sigmaV;
  // Set physical parameters
  m_energyDep = hit->getEnergyDep();
  // Setup geometry information
  setDetectorPlane();
}

PXDRecoHit::PXDRecoHit(const PXDCluster* hit, float sigmaU, float sigmaV, float covUV):
  genfit::PlanarMeasurement(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(0), m_cluster(hit),
  m_energyDep(0)//, m_energyDepError(0)
{
  // Set the sensor UID
  m_sensorID = hit->getSensorID();
  // Set positions
  rawHitCoords_(0) = hit->getU();
  rawHitCoords_(1) = hit->getV();
  // Set the error covariance matrix
  rawHitCov_(0, 0) = sigmaU * sigmaU;
  rawHitCov_(0, 1) = covUV;
  rawHitCov_(1, 0) = covUV;
  rawHitCov_(1, 1) = sigmaV * sigmaV;
  // Set physical parameters
  m_energyDep = hit->getCharge() * Const::ehEnergy;
  //m_energyDepError = 0;
  // Setup geometry information
  setDetectorPlane();
}


PXDRecoHit::PXDRecoHit(const PXDCluster* hit, const genfit::TrackCandHit*):
  genfit::PlanarMeasurement(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(0), m_cluster(hit),
  m_energyDep(0)//, m_energyDepError(0)
{
  // Set the sensor UID
  m_sensorID = hit->getSensorID();
  // Set positions
  rawHitCoords_(0) = hit->getU();
  rawHitCoords_(1) = hit->getV();
  // Set the error covariance matrix
  rawHitCov_(0, 0) = hit->getUSigma() * hit->getUSigma();
  rawHitCov_(0, 1) = hit->getRho() * hit->getUSigma() * hit->getVSigma();
  rawHitCov_(1, 0) = hit->getRho() * hit->getUSigma() * hit->getVSigma();
  rawHitCov_(1, 1) = hit->getVSigma() * hit->getVSigma();
  // Set physical parameters
  m_energyDep = hit->getCharge() * Const::ehEnergy;
  //m_energyDepError = 0;
  // Setup geometry information
  setDetectorPlane();
}

genfit::AbsMeasurement* PXDRecoHit::clone() const
{
  return new PXDRecoHit(*this);
}


void PXDRecoHit::setDetectorPlane()
{
  // Construct a finite detector plane and set it.
  const PXD::SensorInfo& geometry = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));

  // Construct vectors o, u, v
  TVector3 uLocal(1, 0, 0);
  TVector3 vLocal(0, 1, 0);
  TVector3 origin  = geometry.pointToGlobal(TVector3(0, 0, 0), true);
  TVector3 uGlobal = geometry.vectorToGlobal(uLocal, true);
  TVector3 vGlobal = geometry.vectorToGlobal(vLocal, true);

  //Construct the detector plane
  VXD::SensorPlane* finitePlane = new VXD::SensorPlane(m_sensorID, 20.0, 20.0);
  genfit::SharedPlanePtr detPlane(new genfit::DetPlane(origin, uGlobal, vGlobal, finitePlane));
  setPlane(detPlane, m_sensorID);
}


std::vector<genfit::MeasurementOnPlane*> PXDRecoHit::constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const
{
  // Track-based update only takes place when the RecoHit has an associated cluster.
  if (this->getCluster() && PXD::HitCorrector::getInstance().isInitialized()) {
    // Check if we can correct hit coordinates based on track info
    PXDCluster correctedCluster(*this->getCluster());
    const TVectorD& state5 = state.getState();
    correctedCluster = PXD::HitCorrector::getInstance().correctCluster(correctedCluster, state5[1], state5[2]);
    TVectorD hitCoords(2);
    hitCoords(0) = correctedCluster.getU();
    hitCoords(1) = correctedCluster.getV();
    TMatrixDSym hitCov(2);
    hitCov(0, 0) = correctedCluster.getUSigma() * correctedCluster.getUSigma();
    hitCov(0, 1) = correctedCluster.getRho() * correctedCluster.getUSigma() * correctedCluster.getVSigma();
    hitCov(1, 0) = correctedCluster.getRho() * correctedCluster.getUSigma() * correctedCluster.getVSigma();
    hitCov(1, 1) = correctedCluster.getVSigma() * correctedCluster.getVSigma();
    return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(
                                                      hitCoords, hitCov, state.getPlane(), state.getRep(), this->constructHMatrix(state.getRep())
                                                    ));
  } else {
    // No track-based update
    return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(
                                                      rawHitCoords_, rawHitCov_, state.getPlane(), state.getRep(), this->constructHMatrix(state.getRep())
                                                    ));
  }
}

std::vector<genfit::MeasurementOnPlane*> PXDRecoHit::constructMeasurementsOnPlane2(const genfit::StateOnPlane& state) const
{
  // Track-based update only takes place when the RecoHit has an associated cluster
  if (this->getCluster()) {
    // Check if we can correct position coordinates based on track info
    const TVectorD& state5 = state.getState();
    auto offset = PXD::PXDClusterPositionEstimator::getInstance().getClusterOffset(*this->getCluster(), state5[1], state5[2]);

    if (offset != nullptr) {
      // Found a valid offset, lets apply it
      const Belle2::VxdID& sensorID = (*this->getCluster()).getSensorID();
      const Belle2::PXD::SensorInfo& Info = dynamic_cast<const Belle2::PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
      double posU = Info.getUCellPosition((*this->getCluster()).getUStart());
      double posV = Info.getVCellPosition((*this->getCluster()).getVStart());

      TVectorD hitCoords(2);
      hitCoords(0) = posU + offset->getU();
      hitCoords(1) = posV + offset->getV();
      TMatrixDSym hitCov(2);
      hitCov(0, 0) = offset->getUSigma2();
      hitCov(0, 1) = offset->getUVCovariance();
      hitCov(1, 0) = offset->getUVCovariance();
      hitCov(1, 1) = offset->getVSigma2();
      return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(
                                                        hitCoords, hitCov, state.getPlane(), state.getRep(), this->constructHMatrix(state.getRep())
                                                      ));
    }
  }

  // If we reach here, we can do no better than what we have
  return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(
                                                    rawHitCoords_, rawHitCov_, state.getPlane(), state.getRep(), this->constructHMatrix(state.getRep())
                                                  ));
}



