/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 * Derived from: genfit/GFSpacepointHitPolicy                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/ExtHitPolicy.h>

#include <assert.h>

#include <TMatrixD.h>

#include <GFAbsRecoHit.h>

using namespace Belle2;

ClassImp(ExtHitPolicy)

const std::string ExtHitPolicy::fPolicyName = "ExtHitPolicy";

TMatrixD
ExtHitPolicy::hitCoord(GFAbsRecoHit* hit, const GFDetPlane& plane)
{
  TMatrixD returnMat(2, 1);

  TMatrixD _D(3, 1);
  TVector3 _U;
  TVector3 _V;

  _D[0][0] = (plane.getO())[0];
  _D[1][0] = (plane.getO())[1];
  _D[2][0] = (plane.getO())[2];

  _D *= -1.;
  _D += hit->getRawHitCoord();
  //now the vector _D points from the origin of the plane to the hit point

  _U = plane.getU();
  _V = plane.getV();

  returnMat[0][0] = _D[0][0] * _U[0] + _D[1][0] * _U[1] + _D[2][0] * _U[2];
  returnMat[1][0] = _D[0][0] * _V[0] + _D[1][0] * _V[1] + _D[2][0] * _V[2];
  return returnMat;
}

TMatrixD
ExtHitPolicy::hitCov(GFAbsRecoHit* hit, const GFDetPlane& plane)
{
  TVector3 _U;
  TVector3 _V;

  _U = plane.getU();
  _V = plane.getV();

  TMatrixD rawCov = hit->getRawHitCov();

  TMatrixD jac(6, 2);

  // jac = dF_i/dx_j = s_unitvec * t_unitvec, with s=u,v and t=x,y,z
  jac[0][0] = _U[0];
  jac[1][0] = _U[1];
  jac[2][0] = _U[2];
  jac[0][1] = _V[0];
  jac[1][1] = _V[1];
  jac[2][1] = _V[2];

  TMatrixD jac_orig = jac;
  TMatrixD jac_t = jac.T();

  TMatrixD result = jac_t * (rawCov * jac_orig);
  return  result;
}

const GFDetPlane&
ExtHitPolicy::detPlane(GFAbsRecoHit*, const GFAbsTrackRep*)
{
  return fPlane;
}

void
ExtHitPolicy::setDetPlane(const TMatrixD& phasespacePoint)
{
  TVector3 pos(phasespacePoint[0][0], phasespacePoint[1][0], phasespacePoint[2][0]);
  TVector3 dir(phasespacePoint[2][0], phasespacePoint[3][0], phasespacePoint[4][0]);
  dir.SetMag(1.0);
  fPlane.setO(pos);
  fPlane.setNormal(dir);
}
