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

#include <tracking/karlsruhe/AxialTrackFinder.h>
#include <cdc/geocdc/CDCGeometryPar.h>

#include <cmath>
#include <TMath.h>

using namespace std;
using namespace Belle2;

ClassImp(CDCTrackHit)

CDCTrackHit::CDCTrackHit()
{
}


CDCTrackHit::CDCTrackHit(HitCDC&hitcdc)
{
  m_layerId = hitcdc.getLayerId();
  m_wireId = hitcdc.getWireId();
  m_leftDriftLength = hitcdc.getLeftDriftLength();
  m_rightDriftLength = hitcdc.getRightDriftLength();
  m_charge = hitcdc.getCharge();
}

CDCTrackHit::~CDCTrackHit()
{
}


CDCTrackHit CDCTrackHit::castTrackHit(HitCDC aHit)
{
  CDCTrackHit aTrackHit = static_cast<CDCTrackHit>(aHit);
  aTrackHit.setSuperlayerId();
  aTrackHit.setWirePosition();
  aTrackHit.setConformalPosition(0., 0.); //dummy start value
  aTrackHit.setIsUsed(false);
  return aTrackHit;
}

void CDCTrackHit::setSuperlayerId()
{
//Hardcoded CDC Geometry, first and last superlayer have 8 layers, others have 6 layers
  if (m_layerId >= 0 && m_layerId < 8) {
    m_superlayerId = 1;
  }

  else if (m_layerId >= 8 && m_layerId < 50) {
    m_superlayerId = (m_layerId - 2) / 6 + 1 ;
  }

  else if (m_layerId >= 50 && m_layerId < 58) {
    m_superlayerId = 9;
  }

//Check if the superlayer is axial or stereo
  if (m_superlayerId % 2 == 0) {
    m_isAxial = false;
  } else {
    m_isAxial = true;
  }


}

void CDCTrackHit::setWirePosition()
{
  //Get the position of the hit wire from CDCGeometryParameters
  CDCGeometryPar * cdcgp = CDCGeometryPar::Instance();
  CDCGeometryPar & cdcg(*cdcgp);

  //center of the wire
  m_wirePosition.SetX((cdcg.wireForwardPosition(m_layerId, m_wireId).x() + cdcg.wireBackwardPosition(m_layerId, m_wireId).x()) / 2);
  m_wirePosition.SetY((cdcg.wireForwardPosition(m_layerId, m_wireId).y() + cdcg.wireBackwardPosition(m_layerId, m_wireId).y()) / 2);
  m_wirePosition.SetZ((cdcg.wireForwardPosition(m_layerId, m_wireId).z() + cdcg.wireBackwardPosition(m_layerId, m_wireId).z()) / 2);

}

void CDCTrackHit::setIsUsed(bool isUsed)
{
  m_isUsed = isUsed;
}


void CDCTrackHit::setConformalPosition(double conformalX, double conformalY)
{
  m_conformalX = conformalX;
  m_conformalY = conformalY;
}


double CDCTrackHit::getPhi() const
{
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


void CDCTrackHit::shiftAlongZ(TVector3 trackDirection, CDCTrackHit trackHit)
{
  //Get the necessary position of the hit wire from CDCGeometryParameters
  CDCGeometryPar * cdcgp = CDCGeometryPar::Instance();
  CDCGeometryPar & cdcg(*cdcgp);

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
  //B2INFO("Wire vector: "<<wireVector.x()<<"  "<<wireVector.y()<<"  "<<wireVector.z());

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
  double posZ;

  double confX;  //position of the hit on the wire in the conformal plane
  double confY;

  for (int i = 0; i < 101; i++) {  //loop over the parameter vector ( = loop over the lenght of the wire)

    //new point along the wire
    posX = forwardWirePoint.x() + parameter[i] * wireVector.x();
    posY = forwardWirePoint.y() + parameter[i] * wireVector.y();
    posZ = forwardWirePoint.z() + parameter[i] * wireVector.z();

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


