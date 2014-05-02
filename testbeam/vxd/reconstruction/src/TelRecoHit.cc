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
#include <testbeam/vxd/reconstruction/TelRecoHit.h>
#include <testbeam/vxd/dataobjects/TelTrueHit.h>
#include <testbeam/vxd/dataobjects/TelCluster.h>
#include <testbeam/vxd/geometry/SensorInfo.h>
#include <vxd/geometry/SensorPlane.h>
#include <vxd/geometry/GeoCache.h>

#include <genfit/DetPlane.h>
#include <TVector3.h>
#include <TRandom.h>

using namespace std;
using namespace Belle2;

ClassImp(TelRecoHit)

TelRecoHit::TelRecoHit():
  genfit::PlanarMeasurement(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(0), m_cluster(0),
  m_energyDep(0)//, m_energyDepError(0)
{}

TelRecoHit::TelRecoHit(const TelTrueHit* hit, const genfit::TrackCandHit*, float sigmaU, float sigmaV):
  genfit::PlanarMeasurement(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(hit), m_cluster(0),
  m_energyDep(0)//, m_energyDepError(0)
{
  if (!gRandom) B2FATAL("gRandom not initialized, please set up gRandom first");

  // Set the sensor UID
  m_sensorID = hit->getSensorID();

  //If no error is given, estimate the error by dividing the pixel size by sqrt(12)
  if (sigmaU < 0 || sigmaV < 0) {
    const TEL::SensorInfo& geometry = dynamic_cast<const TEL::SensorInfo&>(VXD::GeoCache::get(m_sensorID));
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

TelRecoHit::TelRecoHit(const TelCluster* hit, float sigmaU, float sigmaV, float covUV):
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


TelRecoHit::TelRecoHit(const TelCluster* hit, const genfit::TrackCandHit*):
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

genfit::AbsMeasurement* TelRecoHit::clone() const
{
  return new TelRecoHit(*this);
}


void TelRecoHit::setDetectorPlane()
{
  // Construct a finite detector plane and set it.
  const TEL::SensorInfo& geometry = dynamic_cast<const TEL::SensorInfo&>(VXD::GeoCache::get(m_sensorID));

  // Construct vectors o, u, v
  TVector3 origin  = geometry.pointToGlobal(TVector3(0, 0, 0));
  TVector3 uGlobal = geometry.vectorToGlobal(TVector3(1, 0, 0));
  TVector3 vGlobal = geometry.vectorToGlobal(TVector3(0, 1, 0));

  //Construct the detector plane
  genfit::DetPlane* p = new genfit::DetPlane(origin, uGlobal, vGlobal, new VXD::SensorPlane(m_sensorID, 20, 20));
  genfit::SharedPlanePtr detPlane(p);
  setPlane(detPlane, m_sensorID);
}


std::vector<genfit::MeasurementOnPlane*> TelRecoHit::constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const
{
  return std::vector<genfit::MeasurementOnPlane*>(1, new genfit::MeasurementOnPlane(rawHitCoords_, rawHitCov_, state.getPlane(), state.getRep(), this->constructHMatrix(state.getRep())));
}
