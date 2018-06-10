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
    B2FATAL("Error in SVDRecoHit2D: Incorrect SVDCluster instances on input!");

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
    B2FATAL("Error in SVDRecoHit2D: Incorrect SVDCluster instances on input!");

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
  TVector3 origin  = geometry.pointToGlobal(TVector3(0, 0, 0), true);
  TVector3 uGlobal = geometry.vectorToGlobal(TVector3(1, 0, 0), true);
  TVector3 vGlobal = geometry.vectorToGlobal(TVector3(0, 1, 0), true);

  //Construct the detector plane
  genfit::SharedPlanePtr detPlane(new genfit::DetPlane(origin, uGlobal, vGlobal, new VXD::SensorPlane(m_sensorID, 20, 20)));
  setPlane(detPlane, m_sensorID);
}

std::vector<genfit::MeasurementOnPlane*> SVDRecoHit2D::constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const
{
  // Testing
  bool applyDeformation(true);
  auto sensor = m_sensorID;

  if (applyDeformation) {
    double u = rawHitCoords_[0];
    double v = rawHitCoords_[1];
    double du_dw = state.getState()[1]; // u'
    double dv_dw = state.getState()[2]; // v'

    double p0 = 0;
    double p1 = 0;
    double p2 = 0;
    double p3 = 0;
    double p4 = 0;
    double p5 = 0;
    double p6 = 0;

    auto L1 = [](double x) {return x;};
    auto L2 = [](double x) {return 3 * x * x - 1;};
    auto L3 = [](double x) {return 5 * x * x * x - 3 * x;};

    if (sensor == VxdID("3.7.1")) {
      p0 = -20.308;
      p1 = 26.9969;
      p2 = -9.97735;
      p3 = 38.0269;
      p4 = -9.15816;
      p5 = 6.45265;
      p6 = 5.45303;

    }

    auto x = u;
    auto y = v;
    double dw = p0 * L2(x) + p1 * L1(x) * L1(y) + p2 * L2(y) + p3 * L3(x) + p4 * L2(x) * L1(y) + p5 * L1(x) * L2(y) + p6 * L3(
                  y); // x = u; y = v

    double du = dw * du_dw;
    double dv = dw * dv_dw;

    //TODO: sign?
    TVectorD new_uv(2);
    new_uv[0] = u - du;
    new_uv[1] = v - dv;

    return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(new_uv, rawHitCov_, state.getPlane(),
                                                    state.getRep(), this->constructHMatrix(state.getRep())));

  }
  return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(rawHitCoords_, rawHitCov_, state.getPlane(),
                                                  state.getRep(), this->constructHMatrix(state.getRep())));
}

genfit::AbsMeasurement* SVDRecoHit2D::clone() const
{
  return new SVDRecoHit2D(*this);
}
