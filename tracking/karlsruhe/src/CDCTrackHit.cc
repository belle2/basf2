/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                   *
 * Copyright(C) 2010 - Belle II Collaboration                       *
 *                                    *
 * Author: The Belle II Collaboration                     *
 * Contributors: Oksana Brovchenko                          *
 *                                    *
 * This software is provided "as is" without any warranty.          *
**************************************************************************/


#include "../include/CDCTrackHit.h"

#include <cdc/geocdc/CDCGeometryPar.h>
#include "CLHEP/Geometry/Vector3D.h"

#define pi 3.141592654


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

  else if (m_layerId >= 8 && m_layerId < 14) {
    m_superlayerId = 2;
  }

  else if (m_layerId >= 14 && m_layerId < 20) {
    m_superlayerId = 3;
  }

  else if (m_layerId >= 20 && m_layerId < 26) {
    m_superlayerId = 4;
  }

  else if (m_layerId >= 26 && m_layerId < 32) {
    m_superlayerId = 5;
  }

  else if (m_layerId >= 32 && m_layerId < 38) {
    m_superlayerId = 6;
  }

  else if (m_layerId >= 38 && m_layerId < 44) {
    m_superlayerId = 7;
  }

  else if (m_layerId >= 44 && m_layerId < 50) {
    m_superlayerId = 8;
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
  m_posWireX = (cdcg.wireForwardPosition(m_layerId, m_wireId).x() + cdcg.wireBackwardPosition(m_layerId, m_wireId).x()) / 2;
  m_posWireY = (cdcg.wireForwardPosition(m_layerId, m_wireId).y() + cdcg.wireBackwardPosition(m_layerId, m_wireId).y()) / 2;
  m_posWireZ = (cdcg.wireForwardPosition(m_layerId, m_wireId).z() + cdcg.wireBackwardPosition(m_layerId, m_wireId).z()) / 2;

  //forward end of the wire
  m_posWireX_f = cdcg.wireForwardPosition(m_layerId, m_wireId).x();
  m_posWireY_f = cdcg.wireForwardPosition(m_layerId, m_wireId).y();
  m_posWireZ_f = cdcg.wireForwardPosition(m_layerId, m_wireId).z();

  //backward end of the wire
  m_posWireX_b = cdcg.wireBackwardPosition(m_layerId, m_wireId).x();
  m_posWireY_b = cdcg.wireBackwardPosition(m_layerId, m_wireId).y();
  m_posWireZ_b = cdcg.wireBackwardPosition(m_layerId, m_wireId).z();

  //double norm = sqrt((m_posWireX_b - m_posWireX_f) * (m_posWireX_b - m_posWireX_f) + (m_posWireY_b - m_posWireY_f) * (m_posWireY_b - m_posWireY_f));
  //Direction of the wire
  m_wireVector.SetX((m_posWireX_b - m_posWireX_f));
  m_wireVector.SetY((m_posWireY_b - m_posWireY_f));
  m_wireVector.SetZ((m_posWireZ_b - m_posWireZ_f));

}

void CDCTrackHit::setIsUsed(bool isUsed) { m_isUsed = isUsed;  }


void CDCTrackHit::setConformalPosition(double conformalX, double conformalY)
{
  m_conformalX = conformalX;
  m_conformalY = conformalY;
}


double CDCTrackHit::getPhi() const
{
  double phi = atan(m_posWireY / m_posWireX);

//distribute the phi values from 0 to 2pi
  if (m_posWireX >= 0 && m_posWireY >= 0) {
    phi = atan(m_posWireY / m_posWireX);
  }

  if (m_posWireX < 0) {
    phi = pi + atan(m_posWireY / m_posWireX);
  }

  if (m_posWireX >= 0 && m_posWireY < 0) {
    phi = 2 * pi + atan(m_posWireY / m_posWireX);
  }


  return phi;

}

void CDCTrackHit::setStereoPosition(TVector3 position)
{
  m_posWireX = position.x();
  m_posWireY = position.y();
  m_posWireZ = position.z();
}

int CDCTrackHit::shiftAlongZ(TVector3 trackDirection, CDCTrackHit trackHit)
{
  TVector3 StereoHitPos;   //conformal position of this hit
  TVector3 TrackHitPos;    //conformal position of the given TrackHit (starting point for the "track straight line")
  TVector3 perpPoint;      //intersection from "track line" with a to it perpendicular line through this hit
  TVector3 perpDir;        //direction perpendicular to track direction

  double distance;     //distance between the hit und the intersection point ( = shortest distance from hit to "track line")
  double distanceMax = 0.3;  //start value for the search

  //create a vector with 10 aquidistant values from 0 to 1 to parametrise the wire vector
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
    posX = m_posWireX_f + parameter[i] * m_wireVector.x();
    posY = m_posWireY_f + parameter[i] * m_wireVector.y();
    posZ = m_posWireZ_f + parameter[i] * m_wireVector.z();

    confX = 2 * posX / (posX * posX + posY * posY);
    confY = 2 * posY / (posX * posX + posY * posY);

    //calculation of the shortest distance between the hit point and the track in the conformal plane
    StereoHitPos.SetX(confX);
    StereoHitPos.SetY(confY);
    StereoHitPos.SetZ(0);

    TrackHitPos.SetX(trackHit.getConformalX());
    TrackHitPos.SetY(trackHit.getConformalY());
    TrackHitPos.SetZ(0);

    perpDir.SetX(1);
    perpDir.SetY(-trackDirection.x() / trackDirection.y());
    perpDir.SetZ(0);

    double factor = ((StereoHitPos.y() - TrackHitPos.y()) * trackDirection.x() + (TrackHitPos.x() - StereoHitPos.x()) * trackDirection.y()) / (trackDirection.y() * (perpDir.x() + perpDir.y()));

    perpPoint.SetX(StereoHitPos.x() + factor*perpDir.x());
    perpPoint.SetY(StereoHitPos.y() + factor*perpDir.y());
    perpPoint.SetZ(0);

    //finally the result
    distance = (StereoHitPos - perpPoint).Mag();

    //search for the wire point which gives the shortest distance
    if (distance < distanceMax) {
      distanceMax = distance;
      bestIndex = i;
    }

  }

//assign the new better wire point as hit position
  TVector3 newPosition;
  double x = m_posWireX_f +  parameter[bestIndex] * m_wireVector.x();
  double y = m_posWireY_f +  parameter[bestIndex] * m_wireVector.y();
  double z = m_posWireZ_f +  parameter[bestIndex] * m_wireVector.z();
  double cx = 2 * x / (x * x + y * y);
  double cy = 2 * y / (x * x + y * y);

  m_posWireX = x;
  m_posWireY = y;
  m_posWireZ = z;
  m_conformalX = cx;
  m_conformalY = cy;

  return bestIndex;
}


