/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>
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

TVectorD SVDRecoHit::applyPlanarDeformation(TVectorD rawHit, std::vector<double> planarParameters,
                                            const genfit::StateOnPlane& state) const
{
  // Legendre parametrization of deformation
  auto L1 = [](double x) {return x;};
  auto L2 = [](double x) {return (3 * x * x - 1) / 2;};
  auto L3 = [](double x) {return (5 * x * x * x - 3 * x) / 2;};
  auto L4 = [](double x) {return (35 * x * x * x * x - 30 * x * x + 3) / 8;};

  const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));

  double u = 0;
  double v = 0;
  double length = 0;
  double width = 0;

  if (m_isU) {
    u = rawHit[0];                 // U coordinate of hit
    v = state.getState()(4);       // V coordinate of hit
    width = geometry.getWidth(v);  // Width of sensor (U side) is function of V (slanted)
    length = geometry.getLength(); // Length of sensor (V side)

    u = u * 2 / width;             // Legendre parametrization required U in (-1, 1)
    v = v * 2 / length;            // Legendre parametrization required V in (-1, 1)

  } else {
    v = rawHit[0];                 // V coordinate of hit
    u = state.getState()(3);       // U coordinate of hit
    length = geometry.getLength(); // Length of sensor (V side) is fuction of V (slanted)
    width = geometry.getWidth(v);  // Width of sensor (U side)

    v = v * 2 / length;            // Legendre parametrization required V in (-1, 1)
    u = u * 2 / width;             // Legendre parametrization required U in (-1, 1)

  }

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

  TVectorD pos(1);

  if (m_isU) {
    pos[0] = u + dw * du_dw;
  } else {
    pos[0] = v + dw * dv_dw;
  }

  return pos;
}

std::vector<genfit::MeasurementOnPlane*> SVDRecoHit::constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const
{
  if (!m_isU || m_rotationPhi == 0.0) {

    // Apply planar deformation to rectangular sensor or V coordinate of slanted sensor
    TVectorD pos = applyPlanarDeformation(rawHitCoords_, VXD::GeoCache::get(m_sensorID).getSurfaceParameters(), state);

    return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(pos, rawHitCov_, state.getPlane(),
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

  // Apply planar deformation to U coordinate of slanted sensor
  TVectorD pos = applyPlanarDeformation(coords, VXD::GeoCache::get(m_sensorID).getSurfaceParameters(), state);

  TMatrixDSym cov(scale * scale * rawHitCov_);

  return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(pos, cov, state.getPlane(), state.getRep(),
                                                  this->constructHMatrix(state.getRep())));
}
