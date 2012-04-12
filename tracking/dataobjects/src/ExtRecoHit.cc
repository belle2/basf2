/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/ExtRecoHit.h>

#include <assert.h>
#include <cmath>

#include <TMatrixD.h>

#include <GFAbsTrackRep.h>
#include <GFDetPlane.h>

using namespace std;
using namespace Belle2;

ClassImp(ExtRecoHit)

ExtRecoHit::ExtRecoHit() : GFRecoHitIfc<ExtHitPolicy> (HIT_DIMENSIONS)
{
}

ExtRecoHit::ExtRecoHit(const TMatrixD& phasespacePoint, const TMatrixD& covariance, ExtHitStatus status)
  : GFRecoHitIfc<ExtHitPolicy> (HIT_DIMENSIONS)
{
  assert((phasespacePoint.GetNrows() == HIT_DIMENSIONS) && (phasespacePoint.GetNcols() == 1));
  assert((covariance.GetNrows() == HIT_DIMENSIONS) && (covariance.GetNcols() == HIT_DIMENSIONS));
  fHitCoord = phasespacePoint;
  fHitCov = covariance;
  fPolicy.setDetPlane(fHitCoord);
  m_status = status;

}

GFAbsRecoHit* ExtRecoHit::clone()
{
  return (GFAbsRecoHit*) new ExtRecoHit(*this);
}

TMatrixD ExtRecoHit::getHMatrix(const GFAbsTrackRep* trackRep)
{
  //Assume that dimension 5 is a good enough cue to indicate RKTrackRep
  assert(trackRep->getDim() == 5);
  TMatrixD hMatrix(HIT_DIMENSIONS, 5);
  hMatrix.Zero();
  // dx/du
  //J_pM[0][3] = u.X();
  //J_pM[1][3] = u.Y();
  //J_pM[2][3] = u.Z();
  // dx/dv
  //J_pM[0][4] = v.X();
  //J_pM[1][4] = v.Y();
  //J_pM[2][4] = v.Z();
  hMatrix[0][0] = 1.;
  hMatrix[1][1] = 1.;
  hMatrix[2][2] = 1.;
  return (hMatrix);

}

