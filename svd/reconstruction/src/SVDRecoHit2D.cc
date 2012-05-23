/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Martin Ritter                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/SensorPlane.h>
#include <vxd/geometry/GeoCache.h>

#include <GFDetPlane.h>
#include <TVector3.h>
#include <TRandom.h>

using namespace std;
using namespace Belle2;

ClassImp(SVDRecoHit2D)

SVDRecoHit2D::SVDRecoHit2D():
  GFRecoHitIfc<GFPlanarHitPolicy> (HIT_DIMENSIONS), m_sensorID(0), m_trueHit(0),
  m_energyDep(0)//, m_energyDepError(0)
{}

SVDRecoHit2D::SVDRecoHit2D(const SVDTrueHit* hit, float sigmaU, float sigmaV):
  GFRecoHitIfc<GFPlanarHitPolicy> (HIT_DIMENSIONS), m_sensorID(0), m_trueHit(hit),
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
  fHitCoord(0, 0) = gRandom->Gaus(hit->getU(), sigmaU);
  fHitCoord(1, 0) = gRandom->Gaus(hit->getV(), sigmaV);
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

void SVDRecoHit2D::setDetectorPlane()
{
  // Construct a finite detector plane and set in the policy class.
  const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));

  // Construct vectors o, u, v
  TVector3 origin  = geometry.pointToGlobal(TVector3(0, 0, 0));
  TVector3 uGlobal = geometry.vectorToGlobal(TVector3(1, 0, 0));
  TVector3 vGlobal = geometry.vectorToGlobal(TVector3(0, 1, 0));

  //Construct the detector plane
  GFDetPlane detPlane(origin, uGlobal, vGlobal, new VXD::SensorPlane(m_sensorID, 1.0, 1.0));
  // Set in policy
  fPolicy.setDetPlane(detPlane);
}

GFAbsRecoHit* SVDRecoHit2D::clone()
{
  return new SVDRecoHit2D(*this);
}

TMatrixD SVDRecoHit2D::getHMatrix(const GFAbsTrackRep*)
{
  TMatrixD hMatrix(2, 5);
  hMatrix.Zero();
  hMatrix(0, 3) = 1;
  hMatrix(1, 4) = 1;
  return (hMatrix);
}
