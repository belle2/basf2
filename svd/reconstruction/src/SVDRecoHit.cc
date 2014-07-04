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
#include <framework/gearbox/Unit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/SensorPlane.h>
#include <vxd/geometry/GeoCache.h>

#include <genfit/DetPlane.h>
#include <TVector3.h>
#include <TRandom.h>
#include <cmath>

using namespace std;
using namespace Belle2;



ClassImp(SVDRecoHit)


SVDRecoHit::SVDRecoHit():
  genfit::PlanarMeasurement(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(0),
  m_cluster(0), m_isU(0), m_energyDep(0), m_rotationPhi(0)
{
  setStripV(!m_isU);
}

SVDRecoHit::SVDRecoHit(const SVDTrueHit* hit, bool uDirection, float sigma):
  genfit::PlanarMeasurement(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(hit),
  m_cluster(0), m_isU(uDirection), m_energyDep(0), m_rotationPhi(0)
{
  setStripV(!m_isU);

  // Smear the coordinate when constructing from a TrueHit.
  if (!gRandom) B2FATAL("gRandom not initialized, please set up gRandom first");

  // Set the sensor UID
  m_sensorID = hit->getSensorID();

  //If no error is given, estimate the error to pitch/sqrt(12)
  if (sigma < 0) {
    const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));
    sigma = (m_isU) ? geometry.getUPitch(hit->getV()) / sqrt(12) : geometry.getVPitch() / sqrt(12);
  }

  // Set positions
  rawHitCoords_(0) = (m_isU) ? gRandom->Gaus(hit->getU(), sigma) : gRandom->Gaus(hit->getV(), sigma);
  // Set the error covariance matrix
  rawHitCov_(0, 0) = sigma * sigma;
  // Set physical parameters
  m_energyDep = hit->getEnergyDep();
  // Setup geometry information
  setDetectorPlane();
}

SVDRecoHit::SVDRecoHit(const SVDCluster* hit, const genfit::TrackCandHit*):
  genfit::PlanarMeasurement(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(0),
  m_cluster(hit), m_energyDep(0), m_rotationPhi(0)
{
  // Set the sensor UID
  m_sensorID = hit->getSensorID();
  m_isU = hit->isUCluster();

  setStripV(!m_isU);

  // Determine if we have a wedge sensor.
  const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));

  bool isWedgeU = m_isU && (geometry.getBackwardWidth() > geometry.getForwardWidth());

  // Set positions
  if (!isWedgeU)
    rawHitCoords_(0) = hit->getPosition();
  else {
    // For u coordinate in a wedge sensor, the position line is not u = const.
    // We have to rotate the coordinate system to achieve this.
    m_rotationPhi = atan2((geometry.getBackwardWidth() - geometry.getForwardWidth()) / geometry.getWidth(0) * hit->getPosition(), geometry.getLength());
    // Set the position in the rotated coordinate frame.
    rawHitCoords_(0) = hit->getPosition() * cos(m_rotationPhi);
  }
  // Set the error covariance matrix (this does not scale with position)
  rawHitCov_(0, 0) = hit->getPositionSigma() * hit->getPositionSigma();
  // Set physical parameters
  m_energyDep = hit->getCharge();
  // Setup geometry information
  setDetectorPlane();
}

void SVDRecoHit::setDetectorPlane()
{
  // Construct a finite detector plane and set it.
  const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));
  bool isWedgeU = m_isU && (geometry.getBackwardWidth() > geometry.getForwardWidth());

  // Construct vectors o, u, v
  TVector3 uLocal(1, 0, 0);
  TVector3 vLocal(0, 1, 0);
  if (isWedgeU) {
    double cosPhi = cos(m_rotationPhi);
    double sinPhi = sin(m_rotationPhi);
    uLocal.SetXYZ(cosPhi, sinPhi, 0);
    vLocal.SetXYZ(-sinPhi, cosPhi, 0);
  }
  TVector3 origin  = geometry.pointToGlobal(TVector3(0, 0, 0));
  TVector3 uGlobal = geometry.vectorToGlobal(uLocal);
  TVector3 vGlobal = geometry.vectorToGlobal(vLocal);

  //Construct the detector plane
  VXD::SensorPlane* finitePlane = new VXD::SensorPlane(m_sensorID, 20.0, 20.0);
  if (isWedgeU) finitePlane->setRotation(m_rotationPhi);
  genfit::SharedPlanePtr detPlane(new genfit::DetPlane(origin, uGlobal, vGlobal, finitePlane));
  setPlane(detPlane, m_sensorID);
}

genfit::AbsMeasurement* SVDRecoHit::clone() const
{
  return new SVDRecoHit(*this);
}


std::vector<genfit::MeasurementOnPlane*> SVDRecoHit::constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const
{
  return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(rawHitCoords_, rawHitCov_, state.getPlane(), state.getRep(), this->constructHMatrix(state.getRep())));
}

TMatrixD SVDRecoHit::derivatives(const genfit::StateOnPlane* sop)
{

  // values for global derivatives
  //TMatrixD derGlobal(2, 6);
  TMatrixD derGlobal(2, 6);
  derGlobal.Zero();

  // track u-slope in local sensor system
  double uSlope = sop->getState()[1];
  // track v-slope in local sensor system
  double vSlope = sop->getState()[2];
  // Predicted track u-position in local sensor system
  double uPos = sop->getState()[3];
  // Predicted track v-position in local sensor system
  double vPos = sop->getState()[4];

  //Global derivatives for alignment in sensor local coordinates

  derGlobal(0, 0) = 1.0;
  derGlobal(0, 1) = 0.0;
  derGlobal(0, 2) = - uSlope;
  derGlobal(0, 3) = vPos * uSlope;
  derGlobal(0, 4) = -uPos * uSlope;
  derGlobal(0, 5) = vPos;

  derGlobal(1, 0) = 0.0;
  derGlobal(1, 1) = 1.0;
  derGlobal(1, 2) = - vSlope;
  derGlobal(1, 3) = vPos * vSlope;
  derGlobal(1, 4) = -uPos * vSlope;
  derGlobal(1, 5) = -uPos;

  return derGlobal;
}

vector< int > SVDRecoHit::labels()
{
  VxdID vxdid(getPlaneId());
  int sensorId = (int) vxdid;
  std::vector<int> labGlobal;
  // sensor label = sensorID * 10, then last digit is label for global derivative for the sensor
  int label = sensorId * 10;

  labGlobal.push_back(label + 1); // du
  labGlobal.push_back(label + 2); // dv
  labGlobal.push_back(label + 3); // dw
  labGlobal.push_back(label + 4); // dalpha
  labGlobal.push_back(label + 5); // dbeta
  labGlobal.push_back(label + 6); // dgamma

  return labGlobal;
}



