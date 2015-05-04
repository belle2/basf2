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

#include <TMath.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;

TrackHit::TrackHit(const CDCHit* hit, int ID)
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
  m_hitUsage = TrackHit::not_used;
  checkHitDriftLength();
}

TrackHit::TrackHit(int hitId, int wireID, double driftLength, double sigmaDriftLength,
                   unsigned short charge, int superlayerId, int layerId, TVector3 wirePosition)
{
  m_zReference = 0;
  m_hitUsage = TrackHit::not_used;
  m_wirePosition = wirePosition;
  performConformalTransformation();
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
    m_hitUsage = TrackHit::background;
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
  Vector2D referenceWirePosition = m_underlayingWireHit->getRefPos2D();

  m_wirePosition.setZ(m_zReference);
  m_wirePosition.setXY(referenceWirePosition);
  performConformalTransformation();
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

bool TrackHit::approach(const TrackCandidate& track)
{
  if (getIsAxial())
    return false;

  //Get the necessary position of the hit wire from CDCGeometryParameters
  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();

  TVector3 forwardWirePoint; //forward end of the wire
  TVector3 backwardWirePoint; //backward end of the wire
  TVector3 wireVector;  //direction of the wire

  //forward end of the wire
  forwardWirePoint.SetX(cdcg.wireForwardPosition(getLayerId(), getWireId()).x());
  forwardWirePoint.SetY(cdcg.wireForwardPosition(getLayerId(), getWireId()).y());
  forwardWirePoint.SetZ(cdcg.wireForwardPosition(getLayerId(), getWireId()).z());

  //backward end of the wire
  backwardWirePoint.SetX(cdcg.wireBackwardPosition(getLayerId(), getWireId()).x());
  backwardWirePoint.SetY(cdcg.wireBackwardPosition(getLayerId(), getWireId()).y());
  backwardWirePoint.SetZ(cdcg.wireBackwardPosition(getLayerId(), getWireId()).z());

  //direction of the wire
  wireVector = forwardWirePoint - backwardWirePoint;

  double distanceMax = 5;  //start value for the search

  //create a vector with 100 aquidistant values from 0 to 1 to parametrise the wire vector
  double parameter[101];
  for (int i = 0; i < 101; i++) {
    parameter[i] = i * 0.01;
  }

  int bestIndex = 0; //index to indicate, which value from the parameter vector is the best

  for (int i = 0; i < 101; i++) {
    //loop over the parameter vector ( = loop over the length of the wire)

    //calculation of the shortest distance between the hit point and the track in the conformal plane
    m_wirePosition.setX(backwardWirePoint.x() + parameter[i] * wireVector.x());
    m_wirePosition.setY(backwardWirePoint.y() + parameter[i] * wireVector.y());

    double distance = track.DistanceTo(
                        *this); //distance between the hit und the intersection point ( = shortest distance from hit to "track line")

    //search for the wire point which gives the shortest distance
    if (distance < distanceMax) {
      distanceMax = distance;
      bestIndex = i;
    }
  }

  //assign the new better wire point as hit position
  double x = backwardWirePoint.x() + parameter[bestIndex] * wireVector.x();
  double y = backwardWirePoint.y() + parameter[bestIndex] * wireVector.y();
  double z = backwardWirePoint.z() + parameter[bestIndex] * wireVector.z();

  m_wirePosition.set(x, y, z);
  performConformalTransformation();

  return true;
}

bool TrackHit::approach2(const TrackCandidate& track)
{
  if (getIsAxial())
    return false;

  //Get the necessary position of the hit wire from CDCGeometryParameters
  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();

  TVector3 forwardWirePoint; //forward end of the wire
  TVector3 backwardWirePoint; //backward end of the wire
  TVector3 wireVector;  //direction of the wire

  //forward end of the wire
  forwardWirePoint.SetX(cdcg.wireForwardPosition(getLayerId(), getWireId()).x());
  forwardWirePoint.SetY(cdcg.wireForwardPosition(getLayerId(), getWireId()).y());
  forwardWirePoint.SetZ(cdcg.wireForwardPosition(getLayerId(), getWireId()).z());

  //backward end of the wire
  backwardWirePoint.SetX(cdcg.wireBackwardPosition(getLayerId(), getWireId()).x());
  backwardWirePoint.SetY(cdcg.wireBackwardPosition(getLayerId(), getWireId()).y());
  backwardWirePoint.SetZ(cdcg.wireBackwardPosition(getLayerId(), getWireId()).z());

  //direction of the wire
  wireVector = forwardWirePoint - backwardWirePoint;

  double hx0 = backwardWirePoint.X();
  double hy0 = backwardWirePoint.Y();

  double vx = wireVector.X();
  double vy = wireVector.Y();

  double tx = track.getXc();
  double ty = track.getYc();

  double r_t = sqrt(tx * tx + ty * ty);
  double r_h = getDriftLength();

  double d = sqrt((getOriginalWirePosition().X() - tx) * (getOriginalWirePosition().X() - tx) + (getOriginalWirePosition().Y() - ty) *
                  (getOriginalWirePosition().Y() - ty));
  double radii;

  if (d > max(r_t, r_h))
    radii = r_t + r_h;
  else
    radii = fabs(r_t - r_h);

  double p = 2. / (vx * vx + vy * vy) * (vx * (hx0 - tx) + vy * (hy0 - ty));
  double q = 1. / (vx * vx + vy * vy) * ((hx0 - tx) * (hx0 - tx) + (hy0 - ty) * (hy0 - ty) - radii * radii);

  double l1 = -1.*p / 2 + sqrt(p * p / 4 - q);
  double l2 = -1.*p / 2 - sqrt(p * p / 4 - q);

  double l;

  if (std::isnan(l1) && std::isnan(l2))
    return false;
  else if (std::isnan(l1))
    l = l2;
  else if (std::isnan(l2))
    l = l1;
  else
    l = fabs(l1 - 0.5) < fabs(l2 - 0.5) ? l1 : l2;

  if (l < 0)
    l = 0;
  else if (l > 1)
    l = 1;

  double x = backwardWirePoint.x() + l * wireVector.x();
  double y = backwardWirePoint.y() + l * wireVector.y();
  double z = backwardWirePoint.z() + l * wireVector.z();

  m_wirePosition.set(x, y, z);
  performConformalTransformation();

  return true;
}
