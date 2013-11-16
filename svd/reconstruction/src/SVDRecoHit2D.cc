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

#include <GFDetPlane.h>
#include <TVector3.h>
#include <TRandom.h>
#include <cmath>

using namespace std;
using namespace Belle2;

ClassImp(SVDRecoHit2D)

const double SVDRecoHit2D::c_HMatrixContent[10] = {0, 0, 0, 1, 0, 0, 0, 0, 0, 1};
const TMatrixD SVDRecoHit2D::c_HMatrix = TMatrixD(HIT_DIMENSIONS, 5, c_HMatrixContent);

SVDRecoHit2D::SVDRecoHit2D():
  GFAbsPlanarHit(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(NULL), m_uCluster(NULL), m_vCluster(NULL),
  m_energyDep(0)
{}

SVDRecoHit2D::SVDRecoHit2D(const SVDTrueHit* hit, float sigmaU, float sigmaV):
  GFAbsPlanarHit(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(hit),
  m_uCluster(NULL), m_vCluster(NULL),
  m_energyDep(0)
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

SVDRecoHit2D::SVDRecoHit2D(const VxdID vxdid, const double u, const double v, double sigmaU, double sigmaV):
  GFAbsPlanarHit(HIT_DIMENSIONS), m_sensorID(vxdid.getID()), m_trueHit(NULL), m_uCluster(NULL), m_vCluster(NULL),
  m_energyDep(0)//, m_energyDepError(0)
{
  //If no error is given, estimate the error by dividing the pixel size by sqrt(12)
  if (sigmaU < 0 || sigmaV < 0) {
    const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));
    sigmaU = geometry.getUPitch(v) / sqrt(12);
    sigmaV = geometry.getVPitch(v) / sqrt(12);
  }
  // Set positions
  fHitCoord(0) = u;
  fHitCoord(1) = v;
  // Set the error covariance matrix
  fHitCov(0, 0) = sigmaU * sigmaU;
  fHitCov(0, 1) = 0;
  fHitCov(1, 0) = 0;
  fHitCov(1, 1) = sigmaV * sigmaV;
  // Setup geometry information
  setDetectorPlane();
}

SVDRecoHit2D::SVDRecoHit2D(const SVDCluster& uHit, const SVDCluster& vHit):
  GFAbsPlanarHit(HIT_DIMENSIONS), m_trueHit(NULL), m_uCluster(&uHit), m_vCluster(&vHit),
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

  fHitCoord[0] = uHit.getPosition() * scaleFactorU;
  fHitCoord[1] = vHit.getPosition();

  double sigmaU = uHit.getPositionSigma() * scaleFactorU;
  double sigmaU_sq = sigmaU * sigmaU;
  double sigmaV = vHit.getPositionSigma();
  double sigmaV_sq = sigmaV * sigmaV;

  m_energyDep = 0.5 * (uHit.getCharge() + vHit.getCharge());

  fHitCov(0, 0) = sigmaV_sq * tan_phi * tan_phi + sigmaU_sq * one_over_cos_phi_sqr;
  fHitCov(0, 1) = sigmaV_sq * tan_phi;
  fHitCov(1, 0) = sigmaV_sq * tan_phi;
  fHitCov(1, 1) = sigmaV_sq;
  // Setup geometry information
  setDetectorPlane();
}

SVDRecoHit2D::SVDRecoHit2D(const SVDRecoHit& uHit, const SVDRecoHit& vHit):
  SVDRecoHit2D(*(uHit.getCluster()), *(vHit.getCluster()))
{}


void SVDRecoHit2D::setDetectorPlane()
{
  // Construct a finite detector plane and set it.
  const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));

  // Construct vectors o, u, v
  TVector3 origin  = geometry.pointToGlobal(TVector3(0, 0, 0));
  TVector3 uGlobal = geometry.vectorToGlobal(TVector3(1, 0, 0));
  TVector3 vGlobal = geometry.vectorToGlobal(TVector3(0, 1, 0));

  //Construct the detector plane
  GFDetPlane detPlane(origin, uGlobal, vGlobal, new VXD::SensorPlane(m_sensorID, 20, 20));
  setDetPlane(detPlane);
}

GFAbsRecoHit* SVDRecoHit2D::clone()
{
  return new SVDRecoHit2D(*this);
}

const TMatrixD& SVDRecoHit2D::getHMatrix(const GFAbsTrackRep*)
{
  return c_HMatrix;
}
