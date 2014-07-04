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
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/SensorPlane.h>
#include <vxd/geometry/GeoCache.h>

#include <genfit/DetPlane.h>
#include <TVector3.h>
#include <TRandom.h>
#include <cmath>

using namespace std;
using namespace Belle2;

ClassImp(SVDRecoHit2D)

SVDRecoHit2D::SVDRecoHit2D():
  genfit::PlanarMeasurement(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(0), m_uCluster(0), m_vCluster(0),
  m_energyDep(0)//, m_energyDepError(0)
{}

SVDRecoHit2D::SVDRecoHit2D(const SVDTrueHit* hit, const genfit::TrackCandHit*, float sigmaU, float sigmaV):
  genfit::PlanarMeasurement(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(hit), m_uCluster(0), m_vCluster(0),
  m_energyDep(0)//, m_energyDepError(0)
{
  if (!gRandom) B2FATAL("gRandom not initialized, please set up gRandom first");

  // Set the sensor UID
  m_sensorID = hit->getSensorID();

  //If no error is given, estimate the error by dividing the pixel size by sqrt(12)
  if (sigmaU < 0 || sigmaV < 0) {
    const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));
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

SVDRecoHit2D::SVDRecoHit2D(VxdID::baseType vxdid, const double u, const double v, double sigmaU, double sigmaV):
  genfit::PlanarMeasurement(HIT_DIMENSIONS), m_sensorID(vxdid), m_trueHit(NULL), m_uCluster(0), m_vCluster(0),
  m_energyDep(0)//, m_energyDepError(0)
{
  //If no error is given, estimate the error by dividing the pixel size by sqrt(12)
  if (sigmaU < 0 || sigmaV < 0) {
    const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));
    sigmaU = geometry.getUPitch(v) / sqrt(12);
    sigmaV = geometry.getVPitch(v) / sqrt(12);
  }
  // Set positions
  rawHitCoords_(0) = u;
  rawHitCoords_(1) = v;
  // Set the error covariance matrix
  rawHitCov_(0, 0) = sigmaU * sigmaU;
  rawHitCov_(0, 1) = 0;
  rawHitCov_(1, 0) = 0;
  rawHitCov_(1, 1) = sigmaV * sigmaV;
  // Setup geometry information
  setDetectorPlane();
}

SVDRecoHit2D::SVDRecoHit2D(const SVDCluster& uHit, const SVDCluster& vHit):
  genfit::PlanarMeasurement(HIT_DIMENSIONS), m_trueHit(NULL), m_uCluster(&uHit), m_vCluster(&vHit),
  m_energyDep(0)
{
  if ((uHit.getRawSensorID() != vHit.getRawSensorID()) || !uHit.isUCluster() || vHit.isUCluster())
    B2FATAL("Error in SVDRecoHit2D: Incorrect SVDCluster instances on input!")

    m_sensorID = uHit.getRawSensorID();

  // Now that we have a v coordinate, we can rescale u.
  const SVD::SensorInfo& info =
    dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));

  double DeltaU =
    (info.getForwardWidth() - info.getBackwardWidth()) / info.getLength() / info.getWidth(0);
  double scaleFactorU = 1 + DeltaU * vHit.getPosition();
  double tan_phi = DeltaU * uHit.getPosition(); // need u at v=0!
  double one_over_cos_phi_sqr = 1 + tan_phi * tan_phi;

  rawHitCoords_[0] = uHit.getPosition() * scaleFactorU;
  rawHitCoords_[1] = vHit.getPosition();

  double sigmaU = uHit.getPositionSigma() * scaleFactorU;
  double sigmaU_sq = sigmaU * sigmaU;
  double sigmaV = vHit.getPositionSigma();
  double sigmaV_sq = sigmaV * sigmaV;

  m_energyDep = 0.5 * (uHit.getCharge() + vHit.getCharge());

  rawHitCov_(0, 0) = sigmaV_sq * tan_phi * tan_phi + sigmaU_sq * one_over_cos_phi_sqr;
  rawHitCov_(0, 1) = sigmaV_sq * tan_phi;
  rawHitCov_(1, 0) = sigmaV_sq * tan_phi;
  rawHitCov_(1, 1) = sigmaV_sq;
  // Setup geometry information
  setDetectorPlane();
}

SVDRecoHit2D::SVDRecoHit2D(const SVDRecoHit& uRecoHit, const SVDRecoHit& vRecoHit):
  genfit::PlanarMeasurement(HIT_DIMENSIONS), m_trueHit(NULL), m_uCluster(0), m_vCluster(0), m_energyDep(0)
{
  const SVDCluster& uHit = *(uRecoHit.getCluster());
  const SVDCluster& vHit = *(vRecoHit.getCluster());
  if ((uHit.getRawSensorID() != vHit.getRawSensorID()) || !uHit.isUCluster() || vHit.isUCluster())
    B2FATAL("Error in SVDRecoHit2D: Incorrect SVDCluster instances on input!")

    m_sensorID = uHit.getRawSensorID();
  m_uCluster = &uHit;
  m_vCluster = &vHit;

  // Now that we have a v coordinate, we can rescale u.
  const SVD::SensorInfo& info =
    dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));

  double DeltaU =
    (info.getForwardWidth() - info.getBackwardWidth()) / info.getLength() / info.getWidth(0);
  double scaleFactorU = 1 + DeltaU * vHit.getPosition();
  double tan_phi = DeltaU * uHit.getPosition(); // need u at v=0!
  double one_over_cos_phi_sqr = 1 + tan_phi * tan_phi;

  rawHitCoords_[0] = uHit.getPosition() * scaleFactorU;
  rawHitCoords_[1] = vHit.getPosition();

  double sigmaU = uHit.getPositionSigma() * scaleFactorU;
  double sigmaU_sq = sigmaU * sigmaU;
  double sigmaV = vHit.getPositionSigma();
  double sigmaV_sq = sigmaV * sigmaV;

  m_energyDep = 0.5 * (uHit.getCharge() + vHit.getCharge());

  rawHitCov_(0, 0) = sigmaV_sq * tan_phi * tan_phi + sigmaU_sq * one_over_cos_phi_sqr;
  rawHitCov_(0, 1) = sigmaV_sq * tan_phi;
  rawHitCov_(1, 0) = sigmaV_sq * tan_phi;
  rawHitCov_(1, 1) = sigmaV_sq;
  // Setup geometry information
  setDetectorPlane();
}



void SVDRecoHit2D::setDetectorPlane()
{
  // Construct a finite detector plane and set it.
  const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));

  // Construct vectors o, u, v
  TVector3 origin  = geometry.pointToGlobal(TVector3(0, 0, 0));
  TVector3 uGlobal = geometry.vectorToGlobal(TVector3(1, 0, 0));
  TVector3 vGlobal = geometry.vectorToGlobal(TVector3(0, 1, 0));

  //Construct the detector plane
  genfit::SharedPlanePtr detPlane(new genfit::DetPlane(origin, uGlobal, vGlobal, new VXD::SensorPlane(m_sensorID, 20, 20)));
  setPlane(detPlane, m_sensorID);
}

std::vector<genfit::MeasurementOnPlane*> SVDRecoHit2D::constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const
{
  return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(rawHitCoords_, rawHitCov_, state.getPlane(), state.getRep(), this->constructHMatrix(state.getRep())));
}

genfit::AbsMeasurement* SVDRecoHit2D::clone() const
{
  return new SVDRecoHit2D(*this);
}

TMatrixD SVDRecoHit2D::derivatives(const genfit::StateOnPlane* sop)
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

vector< int > SVDRecoHit2D::labels()
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


