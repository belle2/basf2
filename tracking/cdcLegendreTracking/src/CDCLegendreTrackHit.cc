/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>

#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/translators/SimpleTDCCountTranslator.h>

#include <cmath>
#include <TMath.h>

#include <iostream>
using namespace std;
using namespace Belle2;
using namespace CDC;

CDCLegendreTrackHit::CDCLegendreTrackHit(CDCHit* hit, int ID) : m_storeID(ID), m_wireId(hit->getIWire())
{
  CDC::SimpleTDCCountTranslator sDriftTimeTranslator;
  m_driftTime = sDriftTimeTranslator.getDriftLength(hit->getTDCCount(), WireID(hit->getID()));
  //FIXME: Provide correct parameters, as soon as driftTimeTranslator supports them
  m_deltaDriftTime = sDriftTimeTranslator.getDriftLengthResolution(-999., WireID(hit->getID()), false, -999., -999.);

  m_charge = hit->getADCCount();

  m_superlayerId = hit->getISuperLayer();

  // for the pattern recognition it is nice to have a unique layerId, maybe one can change it later to avoid confusion
  if (hit->getISuperLayer() == 0)
    m_layerId = hit->getILayer();
  else
    m_layerId = hit->getILayer() + hit->getISuperLayer() * 6 + 2;

  //set if the hit is axial or stereo
  if (m_superlayerId % 2 == 0)
    m_isAxial = true;
  else
    m_isAxial = false;

  //set hit coordinates in normal space and conformal plane
  setWirePosition();
  ConformalTransformation();
  m_wirePositionOrig = m_wirePosition;
}

CDCLegendreTrackHit::CDCLegendreTrackHit(const CDCLegendreTrackHit& rhs)
{
  m_storeID = rhs.m_storeID;
  m_driftTime = rhs.getDriftTime();
  m_charge = rhs.m_charge;
  m_wireId = rhs.getWireId();
  m_superlayerId = rhs.getSuperlayerId();
  m_layerId = rhs.getLayerId();
  m_isAxial = rhs.m_isAxial;
  m_wirePositionOrig = rhs.m_wirePositionOrig;

  //set hit coordinates in normal space and conformal plane
  setWirePosition();
  ConformalTransformation();
}

CDCLegendreTrackHit::~CDCLegendreTrackHit()
{
}

void CDCLegendreTrackHit::setWirePosition()
{
  //Get the position of the hit wire from CDCGeometryParameters
  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();

  TVector3 wireBegin = cdcg.wireForwardPosition(m_layerId, m_wireId);
  TVector3 wireEnd   = cdcg.wireBackwardPosition(m_layerId, m_wireId);

  double fraction = (m_zReference - wireBegin.z()) / (wireEnd.z() - wireBegin.z());

  m_wirePosition.SetZ(m_zReference);
  m_wirePosition.SetX(wireBegin.x() + fraction * (wireEnd.x() - wireBegin.x()));
  m_wirePosition.SetY(wireBegin.y() + fraction * (wireEnd.y() - wireBegin.y()));
}

void CDCLegendreTrackHit::ConformalTransformation()
{
  double x = m_wirePosition.x();
  double y = m_wirePosition.y();

  //transformation of the coordinates from normal to conformal plane
  //this is not the actual wire position but the transformed center of the drift circle
  m_conformalX = 2 * x / (x * x + y * y - m_driftTime * m_driftTime);
  m_conformalY = 2 * y / (x * x + y * y - m_driftTime * m_driftTime);

  //conformal drift time =  (x * x + y * y - m_driftTime * m_driftTime)
  m_conformalDriftTime = 2 * m_driftTime / (x * x + y * y - m_driftTime * m_driftTime);

}

double CDCLegendreTrackHit::getPhi() const
{
  //the phi angle of the hit depends on the definition, so I try to use the wireId instead
  //however maybe this function might also be still useful...
  double phi = atan(m_wirePosition.y() / m_wirePosition.x());

  //distribute the phi values from 0 to 2pi
  if (m_wirePosition.x() >= 0 && m_wirePosition.y() >= 0) {
    phi = atan(m_wirePosition.y() / m_wirePosition.x());
  }

  if (m_wirePosition.x() < 0) {
    phi = TMath::Pi() + atan(m_wirePosition.y() / m_wirePosition.x());
  }

  if (m_wirePosition.x() >= 0 && m_wirePosition.y() < 0) {
    phi = 2 * TMath::Pi() + atan(m_wirePosition.y() / m_wirePosition.x());
  }

  return phi;

}

int CDCLegendreTrackHit::getCurvatureSignWrt(double xc, double yc) const
{
  double PI = 3.1415926535897932384626433832795;
  double phi_diff = atan2(yc, xc) - getPhi();

  while (phi_diff > 2 * PI)
    phi_diff -= 2 * PI;
  while (phi_diff < 0)
    phi_diff += 2 * PI;


  if (phi_diff > PI)
    return CDCLegendreTrackCandidate::charge_positive;
  else
    return CDCLegendreTrackCandidate::charge_negative;
}

bool CDCLegendreTrackHit::approach(const CDCLegendreTrackCandidate& track)
{
  if (m_isAxial)
    return false;

  //Get the necessary position of the hit wire from CDCGeometryParameters
  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();

  TVector3 forwardWirePoint; //forward end of the wire
  TVector3 backwardWirePoint; //backward end of the wire
  TVector3 wireVector;  //direction of the wire

  //forward end of the wire
  forwardWirePoint.SetX(cdcg.wireForwardPosition(m_layerId, m_wireId).x());
  forwardWirePoint.SetY(cdcg.wireForwardPosition(m_layerId, m_wireId).y());
  forwardWirePoint.SetZ(cdcg.wireForwardPosition(m_layerId, m_wireId).z());

  //backward end of the wire
  backwardWirePoint.SetX(cdcg.wireBackwardPosition(m_layerId, m_wireId).x());
  backwardWirePoint.SetY(cdcg.wireBackwardPosition(m_layerId, m_wireId).y());
  backwardWirePoint.SetZ(cdcg.wireBackwardPosition(m_layerId, m_wireId).z());

  //direction of the wire
  wireVector = forwardWirePoint - backwardWirePoint;

  double distance; //distance between the hit und the intersection point ( = shortest distance from hit to "track line")
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
    m_wirePosition.SetX(backwardWirePoint.x() + parameter[i] * wireVector.x());
    m_wirePosition.SetY(backwardWirePoint.y() + parameter[i] * wireVector.y());

    distance = track.DistanceTo(*this);

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

  m_wirePosition.SetXYZ(x, y, z);
  ConformalTransformation();

  return true;
}

bool CDCLegendreTrackHit::approach2(const CDCLegendreTrackCandidate& track)
{
  if (m_isAxial)
    return false;

  //Get the necessary position of the hit wire from CDCGeometryParameters
  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();

  TVector3 forwardWirePoint; //forward end of the wire
  TVector3 backwardWirePoint; //backward end of the wire
  TVector3 wireVector;  //direction of the wire

  //forward end of the wire
  forwardWirePoint.SetX(cdcg.wireForwardPosition(m_layerId, m_wireId).x());
  forwardWirePoint.SetY(cdcg.wireForwardPosition(m_layerId, m_wireId).y());
  forwardWirePoint.SetZ(cdcg.wireForwardPosition(m_layerId, m_wireId).z());

  //backward end of the wire
  backwardWirePoint.SetX(cdcg.wireBackwardPosition(m_layerId, m_wireId).x());
  backwardWirePoint.SetY(cdcg.wireBackwardPosition(m_layerId, m_wireId).y());
  backwardWirePoint.SetZ(cdcg.wireBackwardPosition(m_layerId, m_wireId).z());

  //direction of the wire
  wireVector = forwardWirePoint - backwardWirePoint;

  double hx0 = backwardWirePoint.X();
  double hy0 = backwardWirePoint.Y();

  double vx = wireVector.X();
  double vy = wireVector.Y();

  double tx = track.getXc();
  double ty = track.getYc();

  double r_t = sqrt(tx * tx + ty * ty);
  double r_h = m_driftTime;

  double d = sqrt((m_wirePositionOrig.X() - tx) * (m_wirePositionOrig.X() - tx) + (m_wirePositionOrig.Y() - ty) * (m_wirePositionOrig.Y() - ty));
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

  m_wirePosition.SetXYZ(x, y, z);
  ConformalTransformation();

  return true;
}
