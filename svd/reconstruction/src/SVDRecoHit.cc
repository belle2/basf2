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
  rawHitCoords_(0) = hit->getPosition();
  if (isWedgeU) {
    // For u coordinate in a wedge sensor, the position line is not u = const.
    // We have to rotate the coordinate system to achieve this.
    m_rotationPhi = atan2((geometry.getBackwardWidth() - geometry.getForwardWidth()) / geometry.getWidth(0) * hit->getPosition(),
                          geometry.getLength());
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
  TVector3 origin  = geometry.pointToGlobal(TVector3(0, 0, 0), true);
  TVector3 uGlobal = geometry.vectorToGlobal(uLocal, true);
  TVector3 vGlobal = geometry.vectorToGlobal(vLocal, true);

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

  if (!m_isU || m_rotationPhi == 0.0) {
    // Testing
    bool applyDeformation(true);
    auto sensor = m_sensorID;

    if (applyDeformation) {
      double u = 0;
      double v = 0;

      if (m_isU) {
        u = rawHitCoords_(0);
        v = state.getState()(4);
      } else {
        v = rawHitCoords_(0);
        u = state.getState()(3);
      }
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

      if (sensor == VxdID("3.3.2")) {
        p0 = 34.6573;
        p1 = -35.5098;
        p2 = 16.2105;
        p3 = -46.0775;
        p4 = -8.6612;
        p5 = -22.0794;
        p6 = 1.75536;

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

      TVectorD new_pos(1);
      if (m_isU) {
        new_pos[0] = new_uv[0];
      } else {
        new_pos[0] = new_uv[1];
      }

      return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(new_pos, rawHitCov_, state.getPlane(),
                                                      state.getRep(), this->constructHMatrix(state.getRep())));

    }
    return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(rawHitCoords_, rawHitCov_, state.getPlane(),
                                                    state.getRep(), this->constructHMatrix(state.getRep())));
  }

  // Wedged sensor: the measured coordinate in U depends on V and the
  // rotation angle.  Namely, it needs to be scaled.
  double u = rawHitCoords_(0);
  double v = state.getState()(4);
  double uPrime = u - v * tan(m_rotationPhi);
  double scale = uPrime / u;

  TVectorD coords(1);
  coords(0) = uPrime;

  TMatrixDSym cov(scale * scale * rawHitCov_);

  return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(coords, cov, state.getPlane(), state.getRep(),
                                                  this->constructHMatrix(state.getRep())));
}
