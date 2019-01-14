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
#include <vxd/geometry/SensorInfoBase.h>

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

/**********************************************/
/* Applying planar deformation of SVD sensors */
/*  Contributors: Tadeas Bilka, Jakub Kandra  */
/**********************************************/

TVectorD SVDRecoHit::applyPlanarDeformation(std::vector<double> planarParameters, const genfit::StateOnPlane& state) const
{
  // Legendre parametrization of deformation
  auto L1 = [](double x) {return x;};
  auto L2 = [](double x) {return (3 * pow(x, 2) - 1) / 2;};
  auto L3 = [](double x) {return (5 * pow(x, 3) - 3 * x) / 2;};
  auto L4 = [](double x) {return (35 * pow(x, 4) - 30 * pow(x, 2) + 3) / 8;};

  const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));

  double u = 0;
  double v = 0;
  double length = 0;
  double width = 0;

  if (m_isU) {
    u = rawHitCoords_(0);
    v = state.getState()(4);
    width = geometry.getWidth(v);  // Width of sensor (U side) is function of V (slanted)
    length = geometry.getLength(); // Length of sensor (V side)

    u = u * 2 / width;             // Legendre parametrization required U in (-1, 1)
    v = v * 2 / length;            // Legendre parametrization required V in (-1, 1)

  } else {
    v = rawHitCoords_(0);
    u = state.getState()(3);
    length = geometry.getLength(); // Length of sensor (V side) is fuction of V (slanted)
    width = geometry.getWidth(v);  // Width of sensor (U side)

    v = v * 2 / length;            // Legendre parametrization required V in (-1, 1)
    u = u * 2 / width;             // Legendre parametrization required U in (-1, 1)

  }

  if (abs(u) > 1.0 or abs(v) > 1.0) {
    B2WARNING("The hit or his extrapolation is outside of sensor.");
  }

  // Planar deformation using Legendre parametrization
  double dw =
    /* 1st level of deformation: */ planarParameters[0] * L2(u) + planarParameters[1] * L1(u) * L1(v) + planarParameters[2] * L2(v) +
    /* 2nd level of deformation: */ planarParameters[3] * L3(u) + planarParameters[4] * L2(u) * L1(v) + planarParameters[5] * L1(
      u) * L2(v) + planarParameters[6] * L3(v) +
    /* 3rd level of deformation: */ planarParameters[7] * L4(u) + planarParameters[8] * L3(u) * L1(v) + planarParameters[9] * L2(
      u) * L2(v) + planarParameters[10] * L1(u) * L3(v) + planarParameters[11] * L4(v);

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

/**********************************************/
/*       The function of applying planar      */
/*  deformation of SVD sensors are finished.  */
/**********************************************/

std::vector<genfit::MeasurementOnPlane*> SVDRecoHit::constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const
{

  if (!m_isU || m_rotationPhi == 0.0) {

    bool applyDeformation(true); // To apply planar deformation

    if (applyDeformation and m_sensorID == VxdID("4.3.2")) { // Choose testing SVD sensor

      std::vector<double> planarParameters;
      string fileName = "planarParammeters";
      ifstream file;
      file.open(fileName.c_str());
      string line;

      if (file.is_open()) {
        for (int n = 0; getline(file, line) && n < 4; n++) {
          if (n == 3) {
            stringstream ss_line(line);
            while (!ss_line.fail()) {
              double d;
              ss_line >> d;
              if (!ss_line.fail()) planarParameters.push_back(d);
            }
          }
        }
        file.close();
      }

      TVectorD pos = applyPlanarDeformation(planarParameters, state);

      //std::cout << rawHitCoords_(0) << " " << pos(0) << "\n" << "\n";

      return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(pos, rawHitCov_, state.getPlane(),
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
