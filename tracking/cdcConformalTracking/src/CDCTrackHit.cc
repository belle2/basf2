/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCTrackHit.h"

#include <tracking/cdcConformalTracking/AxialTrackFinder.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <cmath>
#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

ClassImp(CDCTrackHit)

CDCTrackHit::CDCTrackHit()
{
  m_storeIndex = 0;
  m_driftTime = 0.0;
  m_charge = 0;
  m_wireId = 0;
  m_superlayerId = 0;
  m_layerId = 0;
  m_tempCellState = 0;
  m_cellState = 0;
  m_conformalX = 0.0;
  m_conformalY = 0.0;
  m_isAxial = true;
}


CDCTrackHit::CDCTrackHit(CDCHit* hit, int index)
{

  m_storeIndex = index;
  m_driftTime = hit->getTDCCount();
  m_charge = hit->getADCCount();

  m_wireId = hit->getIWire();

  m_superlayerId = hit ->getISuperLayer();

  // for the pattern recognition it is nice to have a unique layerId, maybe one can change it later to avoid confusion
  if (hit->getISuperLayer() == 0)   m_layerId = hit->getILayer();
  else m_layerId = hit->getILayer() + hit->getISuperLayer() * 6 + 2 ;

  //set if the hit is axial or stereo
  if (m_superlayerId % 2 == 0) m_isAxial = true;
  else m_isAxial = false;

  //set hit coordinates in normal space and conformal plane
  setWirePosition();
  ConformalTransformation();

  m_tempCellState = 0;
  m_cellState = 0;

}

CDCTrackHit::~CDCTrackHit()
{
}


void CDCTrackHit::setWirePosition()
{
  //Get the position of the hit wire from CDCGeometryParameters
  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();


  //center of the wire
  m_wirePosition.SetX((cdcg.wireForwardPosition(m_layerId, m_wireId).x() + cdcg.wireBackwardPosition(m_layerId, m_wireId).x()) / 2);
  m_wirePosition.SetY((cdcg.wireForwardPosition(m_layerId, m_wireId).y() + cdcg.wireBackwardPosition(m_layerId, m_wireId).y()) / 2);
  m_wirePosition.SetZ((cdcg.wireForwardPosition(m_layerId, m_wireId).z() + cdcg.wireBackwardPosition(m_layerId, m_wireId).z()) / 2);

}


void CDCTrackHit::ConformalTransformation()
{
  double x = m_wirePosition.x();
  double y = m_wirePosition.y();

  m_conformalX = 2 * x / (x * x + y * y); //transformation of the coordinates from normal to conformal plane
  m_conformalY = 2 * y / (x * x + y * y);

}


void CDCTrackHit::addTrackIndex(const int index)
{
  m_TrackIndices.push_back(index);
}


double CDCTrackHit::getPhi() const
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


void CDCTrackHit::shiftAlongZ(TVector3& trackDirection, CDCTrackHit& trackHit)
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

  //Get the coordinates for distance calculation
  TVector3 StereoHitPos;   //conformal position of this hit
  TVector3 TrackHitPos;    //conformal position of the given TrackHit (starting point for the "track straight line")

  double distance;     //distance between the hit und the intersection point ( = shortest distance from hit to "track line")
  double distanceMax = 0.3;  //start value for the search

  //create a vector with 100 aquidistant values from 0 to 1 to parametrise the wire vector
  double parameter[101];
  for (int i = 0 ; i < 101; i++) {
    parameter[i] = i * 0.01;
  }

  int bestIndex = 0; //index to indicate, which value from the parameter vector is the best

  double posX;  //position of the hit on the wire
  double posY;

  double confX;  //position of the hit on the wire in the conformal plane
  double confY;

  for (int i = 0; i < 101; i++) {  //loop over the parameter vector ( = loop over the length of the wire)

    //new point along the wire
    posX = forwardWirePoint.x() + parameter[i] * wireVector.x();
    posY = forwardWirePoint.y() + parameter[i] * wireVector.y();

    confX = 2 * posX / (posX * posX + posY * posY);
    confY = 2 * posY / (posX * posX + posY * posY);

    //calculation of the shortest distance between the hit point and the track in the conformal plane
    StereoHitPos.SetX(confX);
    StereoHitPos.SetY(confY);
    StereoHitPos.SetZ(0);

    TrackHitPos.SetX(trackHit.getConformalX());
    TrackHitPos.SetY(trackHit.getConformalY());
    TrackHitPos.SetZ(0);

    distance = AxialTrackFinder::ShortestDistance(TrackHitPos, trackDirection, StereoHitPos);

    //search for the wire point which gives the shortest distance
    if (distance < distanceMax) {
      distanceMax = distance;
      bestIndex = i;
    }

  }

  //assign the new better wire point as hit position
  double x = forwardWirePoint.x() +  parameter[bestIndex] * wireVector.x();
  double y = forwardWirePoint.y() +  parameter[bestIndex] * wireVector.y();
  double z = forwardWirePoint.z() +  parameter[bestIndex] * wireVector.z();

  double cx = 2 * x / (x * x + y * y);
  double cy = 2 * y / (x * x + y * y);

  m_wirePosition.SetX(x);
  m_wirePosition.SetY(y);
  m_wirePosition.SetZ(z);
  m_conformalX = cx;
  m_conformalY = cy;


}

void CDCTrackHit::setTempCellState(const int tempCellState)
{
  m_tempCellState = tempCellState;
}

void CDCTrackHit::updateCellState()
{
  m_cellState = m_tempCellState;
}


void CDCTrackHit::setSegmentId(const int segmentId)
{
  bool already = false ;
  //check if there is already the same Id
  //only add the new value if it is not already there
  for (unsigned int i = 0; i < m_segmentIds.size(); i++) {
    if (segmentId == m_segmentIds.at(i)) {
      already = true;
    }
  }
  if (already == false) {
    m_segmentIds.push_back(segmentId);
  }
}

void CDCTrackHit::setSegmentIds(const vector<int>& segmentIds)
{

  for (unsigned int i = 0; i < segmentIds.size(); i++) {
    setSegmentId(segmentIds.at(i));
  }
}

void CDCTrackHit::clearSegmentIds()
{
  m_segmentIds.erase(m_segmentIds.begin(), m_segmentIds.end());
}


