/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/dataobjects/CDCRecoHit.h>

// genfit
#include <genfit/RKTrackRep.h>
#include <genfit/GFDetPlane.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <cmath>
#include <TMath.h>

using namespace std;
using namespace Belle2;



using namespace std;
using namespace Belle2;

ClassImp(CDCRecoHit);

CDCRecoHit::CDCRecoHit()
    : GFRecoHitIfc<GFWireHitPolicy> (m_nParHitRep)
{
}

CDCRecoHit::CDCRecoHit(const CDCHit& cdcHit, const float& resolution)
    : GFRecoHitIfc<GFWireHitPolicy> (m_nParHitRep)
{
  // Get the position of the hit wire from CDCGeometryParameters
  // Maybe we should ask, if Instance could give back a reference directly instead of a pointer?
  CDCGeometryPar * cdcgp = CDCGeometryPar::Instance();
  CDCGeometryPar & cdcg(*cdcgp);

  if (cdcHit.getISuperLayer() == 0) {
    m_layerId = cdcHit.getILayer();
  } else {
    m_layerId = 8 + (cdcHit.getISuperLayer() - 1) * 6 + cdcHit.getILayer();
  }

  m_wireId = cdcHit.getIWire();

  m_superLayerId = cdcHit.getIWire();
  m_subLayerId   = cdcHit.getILayer();

  m_driftTime = cdcHit.getDriftTime();
  m_charge    = cdcHit.getCharge();

  fHitCoord[0][0] = cdcg.wireForwardPosition(m_layerId, m_wireId).x(); // x backward position
  fHitCoord[1][0] = cdcg.wireForwardPosition(m_layerId, m_wireId).y();
  fHitCoord[2][0] = cdcg.wireForwardPosition(m_layerId, m_wireId).z();
  fHitCoord[3][0] = cdcg.wireBackwardPosition(m_layerId, m_wireId).x();
  fHitCoord[4][0] = cdcg.wireBackwardPosition(m_layerId, m_wireId).y();
  fHitCoord[5][0] = cdcg.wireBackwardPosition(m_layerId, m_wireId).z();
  fHitCoord[6][0] = cdcHit.getDriftTime();

  fHitCov[6][6] = resolution;

}

GFAbsRecoHit* CDCRecoHit::clone()
{
  return new CDCRecoHit(*this);
}

TMatrixD CDCRecoHit::getHMatrix(const GFAbsTrackRep* stateVector)
{
  TMatrixD m_hMatrix(1, 5);
  m_hMatrix[0][0] = 0.;
  m_hMatrix[0][1] = 0.;
  m_hMatrix[0][2] = 0.;
  m_hMatrix[0][3] = 1.;
  m_hMatrix[0][4] = 0.;

  //don't check for specific Track Representation at the moment, as RKTrackRep is the only one we are currently using.
  return (m_hMatrix);
}
