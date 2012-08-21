/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko, Bastian Kronenbitter                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>

#include <cdc/geometry/CDCGeometryPar.h>

#include <cmath>
#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace cdc;

ClassImp(CDCLegendreTrackHit)

CDCLegendreTrackHit::CDCLegendreTrackHit()
{
}

CDCLegendreTrackHit::CDCLegendreTrackHit(CDCHit* hit, int index)
{
  m_storeIndex = index;
  m_driftTime = hit->getDriftTime();
  m_charge = hit->getADCCount();

  m_wireId = hit->getIWire();

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

CDCLegendreTrackHit::CDCLegendreTrackHit(const CDCLegendreTrackHit& rhs) : TObject()
{
  m_storeIndex = rhs.getStoreIndex();
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

  m_TrackIndices = rhs.getTrackIndices();
}

CDCLegendreTrackHit::~CDCLegendreTrackHit()
{
}

void CDCLegendreTrackHit::setWirePosition()
{
  //Get the position of the hit wire from CDCGeometryParameters
  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();


  //center of the wire
  m_wirePosition.SetX(
    (cdcg.wireForwardPosition(m_layerId, m_wireId).x()
     + cdcg.wireBackwardPosition(m_layerId, m_wireId).x()) / 2);
  m_wirePosition.SetY(
    (cdcg.wireForwardPosition(m_layerId, m_wireId).y()
     + cdcg.wireBackwardPosition(m_layerId, m_wireId).y()) / 2);
  m_wirePosition.SetZ(
    (cdcg.wireForwardPosition(m_layerId, m_wireId).z()
     + cdcg.wireBackwardPosition(m_layerId, m_wireId).z()) / 2);
}

void CDCLegendreTrackHit::ConformalTransformation()
{
  double x = m_wirePosition.x();
  double y = m_wirePosition.y();

  //transformation of the coordinates from normal to conformal plane
  m_conformalX = 2 * x / (x * x + y * y);
  m_conformalY = 2 * y / (x * x + y * y);

  //correct would be (x * x + y * y - m_driftTime * m_driftTime) but very good approximation
  m_conformalDriftTime = 2 * m_driftTime / (x * x + y * y);
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

void CDCLegendreTrackHit::assignToTrack(int ID)
{
  m_TrackIndices.push_back(ID);
}

void CDCLegendreTrackHit::shiftAlongZ(const CDCLegendreTrackCandidate& track)
{
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
  wireVector = backwardWirePoint - forwardWirePoint;

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

    //new point along the wire
    m_wirePosition.SetX(forwardWirePoint.x() + parameter[i] * wireVector.x());
    m_wirePosition.SetY(forwardWirePoint.y() + parameter[i] * wireVector.y());

    //calculation of the shortest distance between the hit point and the track in the conformal plane
    distance = track.DistanceTo(*this);

    //search for the wire point which gives the shortest distance
    if (distance < distanceMax) {
      distanceMax = distance;
      bestIndex = i;
    }
  }

  //assign the new better wire point as hit position
  double x = forwardWirePoint.x() + parameter[bestIndex] * wireVector.x();
  double y = forwardWirePoint.y() + parameter[bestIndex] * wireVector.y();
  double z = forwardWirePoint.z() + parameter[bestIndex] * wireVector.z();

  double cx = 2 * x / (x * x + y * y);
  double cy = 2 * y / (x * x + y * y);

  m_wirePosition.SetX(x);
  m_wirePosition.SetY(y);
  m_wirePosition.SetZ(z);
  m_conformalX = cx;
  m_conformalY = cy;
}
