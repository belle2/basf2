/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/hits/QuadTreeHitWrapper.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;

void QuadTreeHitWrapper::initializeFromWireHit(const CDCWireHit* wireHit)
{
  assert(wireHit);
  m_cdcWireHit = wireHit;

  performConformalTransformation();
}

void QuadTreeHitWrapper::performConformalTransformation()
{
  const Vector2D& twoDimensionalPosition = m_cdcWireHit->getRefPos2D();
  double dominator = twoDimensionalPosition.normSquared() - m_cdcWireHit->getRefDriftLength() * m_cdcWireHit->getRefDriftLength();

  //transformation of the coordinates from normal to conformal plane
  //this is not the actual wire position but the transformed center of the drift circle
  m_conformalPosition = twoDimensionalPosition.divided(dominator) * 2;

  //conformal drift time =  (x * x + y * y - m_driftTime * m_driftTime)
  m_conformalDriftLength = 2 * m_cdcWireHit->getRefDriftLength() / (dominator);
}


std::tuple<double, double, double> QuadTreeHitWrapper::performConformalTransformWithRespectToPoint(double x0, double y0)
{
  Vector2D twoDimensionalPosition = m_cdcWireHit->getRefPos2D() - Vector2D(x0, y0);
  double dominator = twoDimensionalPosition.normSquared() - m_cdcWireHit->getRefDriftLength() * m_cdcWireHit->getRefDriftLength();

  Vector2D conformalPosition = twoDimensionalPosition.divided(dominator) * 2;
  double conformalDriftLength = 2 * m_cdcWireHit->getRefDriftLength() / (dominator);

  return std::make_tuple(conformalPosition.x(), conformalPosition.y(), conformalDriftLength);
}


bool QuadTreeHitWrapper::checkHitDriftLength()
{
  //Get the position of the hit wire from CDCGeometryParameters
  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();

  Vector3D wireBegin(cdcg.wireForwardPosition(m_cdcWireHit->getWireID().getICLayer(), m_cdcWireHit->getWireID().getIWire()));

  Vector3D wireBeginNeighbor;

  if (m_cdcWireHit->getWireID().getIWire() != 0) {
    wireBeginNeighbor = cdcg.wireForwardPosition(m_cdcWireHit->getWireID().getICLayer(), m_cdcWireHit->getWireID().getIWire() - 1);
  } else {
    wireBeginNeighbor = cdcg.wireForwardPosition(m_cdcWireHit->getWireID().getICLayer(), m_cdcWireHit->getWireID().getIWire() + 1);
  }

  double delta = fabs(Vector3D(wireBegin - wireBeginNeighbor).xy().norm());

  double coef = 1.;

  if (m_cdcWireHit->isAxial()) coef = 0.8;
  else coef = 0.9;


  if (m_cdcWireHit->getRefDriftLength() > delta * coef) {
    return false;
  }

  return true;
}

