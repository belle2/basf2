/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/dataobjects/SVDRecoHit.h>

#include <pxd/geopxd/SiGeoCache.h>

#include <pxd/dataobjects/SiSensorPlane.h>
#include <GFDetPlane.h>

#include <TVector3.h>

using namespace std;
using namespace Belle2;

ClassImp(SVDRecoHit)

SVDRecoHit::SVDRecoHit() :
    GFRecoHitIfc<GFPlanarHitPolicy> (m_nParHitRep), m_sensorUniID(0),
    m_energyDep(0), m_energyDepError(1)
{
  ;
}

SVDRecoHit::SVDRecoHit(const SVDHit* hit) :
    GFRecoHitIfc<GFPlanarHitPolicy> (m_nParHitRep), m_sensorUniID(0),
    m_energyDep(0), m_energyDepError(1)
{
  // Set the sensor UID
  m_sensorUniID = hit->getSensorUniID();

  // Set positions
  fHitCoord(0, 0) = hit->getU();
  fHitCoord(1, 0) = hit->getV();

  // Set the error covariance matrix
  fHitCov(0, 0) = hit->getUError();
  fHitCov(0, 1) = hit->getUVCov();
  fHitCov(1, 0) = hit->getUVCov();
  fHitCov(1, 1) = hit->getVError();

  // Set physical parameters
  m_energyDep = hit->getEnergyDep();
  m_energyDepError = hit->getEnergyDepError();

  // Construct a finite detector plane and set in the policy class.
  SiGeoCache* geometry = SiGeoCache::instance();

  // Construct vectors o, u, v
  TVector3 oLocal(1, 0, 0);
  TVector3 oGlobal(0, 0, 0);
  geometry->localToMasterVec(m_sensorUniID, oLocal, oGlobal);
  TVector3 uLocal(0, 1, 0);
  TVector3 uGlobal(0, 0, 0);
  geometry->localToMasterVec(m_sensorUniID, uLocal, uGlobal);
  TVector3 vLocal(0, 0, 1);
  TVector3 vGlobal(0, 0, 0);
  geometry->localToMasterVec(m_sensorUniID, vLocal, vGlobal);

  // Construct finite plane - we use Trapezoidal plane for SVD
  double du = 0.5 * geometry->getUSensorSize(m_sensorUniID, 0);
  double dv = 0.5 * geometry->getVSensorSize(m_sensorUniID);
  double dudv = geometry->getUSensorSize(m_sensorUniID, dv) - du;
  if (dv > 0) dudv /= dv; else dudv = 0;
  GFDetPlane detPlane(oGlobal, uGlobal, vGlobal, new SiSensorPlane(0, 0, du, dv, dudv));
  // Set in policy
  fPolicy.setDetPlane(detPlane);
}

GFAbsRecoHit* SVDRecoHit::clone()
{
  return new SVDRecoHit(*this);
}

TMatrixD SVDRecoHit::getHMatrix(const GFAbsTrackRep* stateVector)
{
  TMatrixD hMatrix(2, 5);
  hMatrix.Zero();
  hMatrix(0, 3) = 1;
  hMatrix(1, 4) = 1;
  return (hMatrix);
}
