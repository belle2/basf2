/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/TrackHit.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;

TrackHit::TrackHit(const CDCHit* hit, int)
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  initializeFromWireHit(wireHitTopology.getWireHit(hit));
}

void TrackHit::initializeFromWireHit(const CDCWireHit* wireHit)
{
  m_underlayingWireHit = wireHit;
  m_zReference = 0;

  //set hit coordinates in normal space and conformal plane
  setWirePosition();
  performConformalTransformation();
  m_hitUsage = TrackHit::c_notUsed;
  checkHitDriftLength();
}

bool TrackHit::checkHitDriftLength()
{
  //Get the position of the hit wire from CDCGeometryParameters
  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();

  TVector3 wireBegin = cdcg.wireForwardPosition(getLayerId(), getWireId());

  TVector3 wireBeginNeighbor;

  if (getWireId() != 0) {
    wireBeginNeighbor = cdcg.wireForwardPosition(getLayerId(), getWireId() - 1);
  } else {
    wireBeginNeighbor = cdcg.wireForwardPosition(getLayerId(), getWireId() + 1);
  }

//  double delta = sqrt((wireBegin.x() - wireBeginNeighbor.x()) * (wireBegin.x() - wireBeginNeighbor.x()) +
//                      (wireBegin.y() - wireBeginNeighbor.y()) * (wireBegin.y() - wireBeginNeighbor.y()));
  double delta = fabs(TVector3(wireBegin - wireBeginNeighbor).Pt());

  double coef = 1.;

  if (getIsAxial()) coef = 0.8;
  else coef = 0.9;


  if (getDriftLength() > delta * coef) {
    m_hitUsage = TrackHit::c_background;
//    B2INFO("Bad hit!");
    return false;
  }

  return true;
}

/** Assigns the Z coordinate of the hit wire and update XY coordinates*/
void TrackHit::setZReference(double zReference)
{
  m_zReference = zReference;

  setWirePosition();
}

void TrackHit::setWirePosition()
{
  m_wirePosition = calculateFractionedPosition(getZReference());
  performConformalTransformation();
}

TVector3 TrackHit::calculateFractionedPosition(double zReferenz) const
{
  const TVector3& wireForwardPosition = getForwardWirePosition();
  const TVector3& wireBackwardPosition = getBackwardWirePosition();

  double fraction = (zReferenz - wireForwardPosition.z()) / (wireBackwardPosition.z() - wireForwardPosition.z());

  return wireForwardPosition + fraction * (wireBackwardPosition - wireForwardPosition);
}

void TrackHit::performConformalTransformation()
{
  const Vector2D& twoDimensionalPosition = m_wirePosition.xy();
  double dominator = twoDimensionalPosition.normSquared() - getDriftLength() * getDriftLength();

  //transformation of the coordinates from normal to conformal plane
  //this is not the actual wire position but the transformed center of the drift circle
  m_conformalPosition = 2 * twoDimensionalPosition.divided(dominator);

  //conformal drift time =  (x * x + y * y - m_driftTime * m_driftTime)
  m_conformalDriftLength = 2 * getDriftLength() / (dominator);
}


std::tuple<double, double, double> TrackHit::performConformalTransformWithRespectToPoint(double x0, double y0)
{
  Vector2D twoDimensionalPosition = m_wirePosition.xy() - Vector2D(x0, y0);
  double dominator = twoDimensionalPosition.normSquared() - getDriftLength() * getDriftLength();

  Vector2D conformalPosition = 2 * twoDimensionalPosition.divided(dominator);
  double conformalDriftLength = 2 * getDriftLength() / (dominator);

  return std::make_tuple(conformalPosition.x(), conformalPosition.y(), conformalDriftLength);
}

double TrackHit::getPhi() const
{
  //the phi angle of the hit depends on the definition, so I try to use the wireId instead
  //however maybe this function might also be still useful...
  double phi = atan(getWirePosition().y() / getWirePosition().x());

  if (getWirePosition().x() < 0) {
    phi += TMath::Pi();
  }

  if (getWirePosition().x() >= 0 && getWirePosition().y() < 0) {
    phi += 2 * TMath::Pi();
  }

  return phi;
}

int TrackHit::getCurvatureSignWrt(double xc, double yc) const
{
  double phi_diff = atan2(yc, xc) - getPhi();

  while (phi_diff > 2 * TMath::Pi())
    phi_diff -= 2 * TMath::Pi();
  while (phi_diff < 0)
    phi_diff += 2 * TMath::Pi();

  if (phi_diff > TMath::Pi())
    return TrackCandidate::charge_positive;
  else
    return TrackCandidate::charge_negative;
}
