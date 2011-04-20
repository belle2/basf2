/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/dataobjects/PXDRecoHit.h>

#include <pxd/geopxd/SiGeoCache.h>

#include <GFDetPlane.h>
#include <GFRectFinitePlane.h>

#include <TVector3.h>

using namespace std;
using namespace Belle2;

ClassImp(PXDRecoHit)

PXDRecoHit::PXDRecoHit() :
    GFRecoHitIfc<GFPlanarHitPolicy> (m_nParHitRep), m_sensorUniID(0),
    m_energyDep(0), m_energyDepError(1)
{
  ;
}

PXDRecoHit::PXDRecoHit(const PXDHit* hit) :
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
  geometry->localToMasterVec(m_sensorUniID, oLocal, oGlobal);// oGlobal.Print();
  TVector3 uLocal(0, 1, 0);
  TVector3 uGlobal(0, 0, 0);
  geometry->localToMasterVec(m_sensorUniID, uLocal, uGlobal);// uGlobal.Print();
  TVector3 vLocal(0, 0, 1);
  TVector3 vGlobal(0, 0, 0);
  geometry->localToMasterVec(m_sensorUniID, vLocal, vGlobal);// vGlobal.Print();

  // Construct finite plane - we can do with RectFinitPlanes for PXD
  double u1 = -0.5 * geometry->getUSensorSize(m_sensorUniID);
  double u2 = - u1;
  double v1 = -0.5 * geometry->getVSensorSize(m_sensorUniID);
  double v2 = - v1;
  GFDetPlane detPlane(oGlobal, uGlobal, vGlobal, new GFRectFinitePlane(u1, u2, v1, v2));
  // Set in policy
  fPolicy.setDetPlane(detPlane);
}

GFAbsRecoHit* PXDRecoHit::clone()
{
  return new PXDRecoHit(*this);
}

TMatrixD PXDRecoHit::getHMatrix(const GFAbsTrackRep* stateVector)
{
  TMatrixD hMatrix(2, 5);
  hMatrix.Zero();
  hMatrix(0, 3) = 1;
  hMatrix(1, 4) = 1;
  return (hMatrix);
}
