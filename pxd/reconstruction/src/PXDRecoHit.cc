/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <pxd/reconstruction/PXDClusterPositionEstimator.h>
#include <pxd/reconstruction/PXDGainCalibrator.h>
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
  const PXD::SensorInfo& SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));
  auto ADUToEnergy = PXD::PXDGainCalibrator::getInstance().getADUToEnergy(m_sensorID, SensorInfo.getUCellID(hit->getU()),
                     SensorInfo.getVCellID(hit->getV()));
  m_energyDep = hit->getCharge() * ADUToEnergy;
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
  const PXD::SensorInfo& SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));
  auto ADUToEnergy = PXD::PXDGainCalibrator::getInstance().getADUToEnergy(m_sensorID, SensorInfo.getUCellID(hit->getU()),
                     SensorInfo.getVCellID(hit->getV()));
  m_energyDep = hit->getCharge() * ADUToEnergy;
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


float PXDRecoHit::getShapeLikelyhood(const genfit::StateOnPlane& state) const
{
  // We need an associated cluster
  if (this->getCluster()) {
    // Likelyhood depends on the fitted incidence angles into the sensor
    const TVectorD& state5 = state.getState();
    return PXD::PXDClusterPositionEstimator::getInstance().getShapeLikelyhood(*this->getCluster(), state5[1], state5[2]);
  }
  // If we reach here, we can do no better than return zero
  return 0;
}

TVectorD PXDRecoHit::applyPlanarDeformation(TVectorD hitCoords, std::vector<double> planarParameters,
                                            const genfit::StateOnPlane& state) const
{
  // Legendre parametrization of deformation
  auto L1 = [](double x) {return x;};
  auto L2 = [](double x) {return (3 * x * x - 1) / 2;};
  auto L3 = [](double x) {return (5 * x * x * x - 3 * x) / 2;};
  auto L4 = [](double x) {return (35 * x * x * x * x - 30 * x * x + 3) / 8;};

  const PXD::SensorInfo& geometry = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));

  double u = hitCoords[0];
  double v = hitCoords[1];
  double width = geometry.getWidth(v);              // Width of sensor (U side)
  double length = geometry.getLength();             // Length of sensor (V side)
  u = u * 2 / width;                                // Legendre parametrization required U in (-1, 1)
  v = v * 2 / length;                               // Legendre parametrization required V in (-1, 1)

  /* Planar deformation using Legendre parametrization
     w(u, v) = L_{31} * L2(u) + L_{32} * L1(u) * L1(v) + L_{33} * L2(v) +
               L_{41} * L3(u) + L_{42} * L2(u) * L1(v) + L_{43} * L1(u) * L2(v) + L_{44} * L3(v) +
               L_{51} * L4(u) + L_{52} * L3(u) * L1(v) + L_{53} * L2(u) * L2(v) + L_{54} * L1(u) * L3(v) + L_{55} * L4(v); */
  double dw =
    planarParameters[0] * L2(u) + planarParameters[1] * L1(u) * L1(v) + planarParameters[2] * L2(v) +
    planarParameters[3] * L3(u) + planarParameters[4] * L2(u) * L1(v) + planarParameters[5] * L1(u) * L2(v) + planarParameters[6] * L3(
      v) +
    planarParameters[7] * L4(u) + planarParameters[8] * L3(u) * L1(v) + planarParameters[9] * L2(u) * L2(v) + planarParameters[10] * L1(
      u) * L3(v) + planarParameters[11] * L4(v);

  double du_dw = state.getState()[1]; // slope in U direction
  double dv_dw = state.getState()[2]; // slope in V direction

  u = u * width / 2;  // from Legendre to Local parametrization
  v = v * length / 2; // from Legendre to Local parametrization

  TVectorD pos(2);

  pos[0] = u + dw * du_dw;
  pos[1] = v + dw * dv_dw;

  return pos;
}

std::vector<genfit::MeasurementOnPlane*> PXDRecoHit::constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const
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

      // Apply planar deformation
      TVectorD pos = applyPlanarDeformation(hitCoords, VXD::GeoCache::get(m_sensorID).getSurfaceParameters(), state);

      return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(
                                                        pos, hitCov, state.getPlane(), state.getRep(), this->constructHMatrix(state.getRep())
                                                      ));
    }
  }

  // Apply planar deformation
  TVectorD pos = applyPlanarDeformation(rawHitCoords_, VXD::GeoCache::get(m_sensorID).getSurfaceParameters(), state);

  // If we reach here, we can do no better than what we have
  return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(
                                                    pos, rawHitCov_, state.getPlane(), state.getRep(), this->constructHMatrix(state.getRep())
                                                  ));
}



