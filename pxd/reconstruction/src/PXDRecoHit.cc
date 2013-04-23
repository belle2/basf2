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
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/SensorPlane.h>
#include <vxd/geometry/GeoCache.h>

#include <GFDetPlane.h>
#include <TVector3.h>
#include <TRandom.h>

using namespace std;
using namespace Belle2;

ClassImp(PXDRecoHit)

const double PXDRecoHit::c_HMatrixContent[10] = {0, 0, 0, 1, 0, 0, 0, 0, 0, 1};
const TMatrixD PXDRecoHit::c_HMatrix = TMatrixD(HIT_DIMENSIONS, 5, c_HMatrixContent);

PXDRecoHit::PXDRecoHit():
  GFAbsPlanarHit(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(0), m_cluster(0), m_vxdSimpleDigiHit(NULL),
  m_energyDep(0)//, m_energyDepError(0)
{}

PXDRecoHit::PXDRecoHit(const PXDTrueHit* hit, float sigmaU, float sigmaV):
  GFAbsPlanarHit(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(hit), m_cluster(0), m_vxdSimpleDigiHit(NULL),
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
  fHitCoord(0) = gRandom->Gaus(hit->getU(), sigmaU);
  fHitCoord(1) = gRandom->Gaus(hit->getV(), sigmaV);
  // Set the error covariance matrix
  fHitCov(0, 0) = sigmaU * sigmaU;
  fHitCov(0, 1) = 0;
  fHitCov(1, 0) = 0;
  fHitCov(1, 1) = sigmaV * sigmaV;
  // Set physical parameters
  m_energyDep = hit->getEnergyDep();
  // Setup geometry information
  setDetectorPlane();
}

PXDRecoHit::PXDRecoHit(const PXDCluster* hit, float sigmaU, float sigmaV, float covUV):
  GFAbsPlanarHit(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(0), m_cluster(hit), m_vxdSimpleDigiHit(NULL),
  m_energyDep(0)//, m_energyDepError(0)
{
  // Set the sensor UID
  m_sensorID = hit->getSensorID();
  // Set positions
  fHitCoord(0) = hit->getU();
  fHitCoord(1) = hit->getV();
  // Set the error covariance matrix
  fHitCov(0, 0) = sigmaU * sigmaU;
  fHitCov(0, 1) = covUV;
  fHitCov(1, 0) = covUV;
  fHitCov(1, 1) = sigmaV * sigmaV;
  // Set physical parameters
  m_energyDep = hit->getCharge() * Const::ehEnergy;
  //m_energyDepError = 0;
  // Setup geometry information
  setDetectorPlane();
}


PXDRecoHit::PXDRecoHit(const PXDCluster* hit):
  GFAbsPlanarHit(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(0), m_cluster(hit), m_vxdSimpleDigiHit(NULL),
  m_energyDep(0)//, m_energyDepError(0)
{
  // Set the sensor UID
  m_sensorID = hit->getSensorID();
  const PXD::SensorInfo& geometry = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));
  double sigmaU = geometry.getUPitch(hit->getV()) / sqrt(12);
  double sigmaV = geometry.getVPitch(hit->getV()) / sqrt(12);
  // Set positions
  fHitCoord(0) = hit->getU();
  fHitCoord(1) = hit->getV();
  // Set the error covariance matrix
  fHitCov(0, 0) = sigmaU * sigmaU;
  fHitCov(0, 1) = 0;
  fHitCov(1, 0) = 0;
  fHitCov(1, 1) = sigmaV * sigmaV;
  // Set physical parameters
  m_energyDep = hit->getCharge() * Const::ehEnergy;
  //m_energyDepError = 0;
  // Setup geometry information
  setDetectorPlane();
}

PXDRecoHit::PXDRecoHit(const VXDSimpleDigiHit* hit):
  GFAbsPlanarHit(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(0), m_cluster(0), m_vxdSimpleDigiHit(hit),
  m_energyDep(0)//, m_energyDepError(0)
{
  // Set the sensor UID
  m_sensorID = hit->getSensorID();
  double sigmaU = hit->getSigU();
  double sigmaV = hit->getSigV();
  // Set positions
  fHitCoord(0) = hit->getU();
  fHitCoord(1) = hit->getV();
  // Set the error covariance matrix
  fHitCov(0, 0) = sigmaU * sigmaU;
  fHitCov(0, 1) = 0;
  fHitCov(1, 0) = 0;
  fHitCov(1, 1) = sigmaV * sigmaV;
  // Set physical parameters
  //m_energyDep = hit->getCharge() * Const::ehEnergy;
  //m_energyDepError = 0;
  // Setup geometry information
  setDetectorPlane();
}

void PXDRecoHit::setDetectorPlane()
{
  // Construct a finite detector plane and set it.
  const PXD::SensorInfo& geometry = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));

  // Construct vectors o, u, v
  TVector3 origin  = geometry.pointToGlobal(TVector3(0, 0, 0));
  TVector3 uGlobal = geometry.vectorToGlobal(TVector3(1, 0, 0));
  TVector3 vGlobal = geometry.vectorToGlobal(TVector3(0, 1, 0));

  //Construct the detector plane
  GFDetPlane detPlane(origin, uGlobal, vGlobal, new VXD::SensorPlane(m_sensorID, 20, 20));
  setDetPlane(detPlane);
}

GFAbsRecoHit* PXDRecoHit::clone()
{
  return new PXDRecoHit(*this);
}

const TMatrixD& PXDRecoHit::getHMatrix(const GFAbsTrackRep*)
{
  return c_HMatrix;
}
