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

const TMatrixD ExtRecoHit::c_hMatrix = TMatrixD(HIT_DIMENSIONS, 5);

ExtRecoHit::ExtRecoHit(): GFAbsRecoHit(HIT_DIMENSIONS)
{
  ;
}

ExtRecoHit::ExtRecoHit(const TMatrixD& phasespacePoint, const TMatrixD& covariance, ExtHitStatus status): GFAbsRecoHit(HIT_DIMENSIONS)
{
  ;
  assert((phasespacePoint.GetNrows() == HIT_DIMENSIONS) && (phasespacePoint.GetNcols() == 1));
  assert((covariance.GetNrows() == HIT_DIMENSIONS) && (covariance.GetNcols() == HIT_DIMENSIONS));

  fHitCoord = TVectorD(HIT_DIMENSIONS, phasespacePoint.GetMatrixArray());
  fHitCov = TMatrixDSym(HIT_DIMENSIONS, covariance.GetMatrixArray());
  //setDetPlane(fHitCoord);
  m_status = status;

}

GFAbsRecoHit* ExtRecoHit::clone()
{
  return (GFAbsRecoHit*) new ExtRecoHit(*this);
}

const TMatrixD& ExtRecoHit::getHMatrix(const GFAbsTrackRep* trackRep)
{
  return (c_hMatrix);
}

