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
#include <vxd/geometry/SensorInfoBase.h>

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

/**********************************************/
/* Applying planar deformation of SVD sensors */
/*  Contributors: Tadeas Bilka, Jakub Kandra  */
/**********************************************/

TVectorD SVDRecoHit2D::applyPlanarDeformation(std::vector<double> planarParameters, const genfit::StateOnPlane& state) const
{
  // Legendre parametrization of deformation
  auto L1 = [](double x) {return x;};
  auto L2 = [](double x) {return (3 * pow(x, 2) - 1) / 2;};
  auto L3 = [](double x) {return (5 * pow(x, 3) - 3 * x) / 2;};
  auto L4 = [](double x) {return (35 * pow(x, 4) - 30 * pow(x, 2) + 3) / 8;};

  const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));

  double u = rawHitCoords_[0];
  double v = rawHitCoords_[1];
  double width = geometry.getWidth(v);              // Width of sensor (U side)
  double length = geometry.getLength();             // Length of sensor (V side)
  u = u * 2 / width;                                // Legendre parametrization required U in (-1, 1)
  v = v * 2 / length;                               // Legendre parametrization required V in (-1, 1)

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

  TVectorD pos(2);

  pos[0] = u + dw * du_dw;
  pos[1] = v + dw * dv_dw;

  return pos;
}

/**********************************************/
/*       The function of applying planar      */
/*  deformation of SVD sensors are finished.  */
/**********************************************/

std::vector<genfit::MeasurementOnPlane*> SVDRecoHit2D::constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const
{

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

genfit::AbsMeasurement* SVDRecoHit2D::clone() const
{
  return new SVDRecoHit2D(*this);
}
