/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                   *
 * Copyright(C) 2010 - Belle II Collaboration                       *
 *                                    *
 * Author: The Belle II Collaboration                     *
 * Contributors: Oksana Brovchenko                          *
 *                                    *
 * This software is provided "as is" without any warranty.          *
**************************************************************************/

#include "../include/CDCSegment.h"

#include<cmath>
#include<cstdlib>




using namespace std;
using namespace Belle2;

ClassImp(CDCSegment)

CDCSegment::CDCSegment()
{
}

CDCSegment::~CDCSegment()
{
}

CDCSegment::CDCSegment(int superlayerId, int Id)
{
  m_superlayerId = superlayerId;
  m_Id = Id;

  if (m_superlayerId % 2 == 0) {
    m_isAxial = false;
  } else {
    m_isAxial = true;
  }

//Set some start values
  m_nHits = 0;
  m_isUsed = false;
  m_isGood = true;

  m_direction.SetX(0);
  m_direction.SetY(0);
  m_direction.SetZ(0);

}


void CDCSegment::addTrackHit(CDCTrackHit &aTrackHit)
{
  m_TrackHits.push_back(aTrackHit);
  update();
}

void CDCSegment::setIsGood(bool isGood) {m_isGood = isGood;}

void CDCSegment::setIsUsed(bool isUsed)
{

  m_isUsed = isUsed;

  for (int i = 0; i < m_nHits; i++) {
    m_TrackHits[i].setIsUsed(isUsed);
  }
}

void CDCSegment::setTrackCandId(int trackId)
{
  bool already = false ;
  //check if there is already the same Id
  //only add the new value if it is not already there
  for (unsigned int i = 0; i < m_trackCandId.size(); i++) {
    if (trackId == m_trackCandId.at(i)) {
      already = true;
    }
  }
  if (already == false) {
    m_trackCandId.push_back(trackId);
  }
}

void CDCSegment::setTrackCandId(vector<int> trackId)
{

  for (unsigned int i = 0; i < trackId.size(); i++) {
    setTrackCandId(trackId.at(i));
  }
}

void CDCSegment::update()
{
  //Assign correct innerMost and outerMost hits
  m_nHits = m_TrackHits.size();
  int min = 0; //index for the innermost Hit
  int max = 0; //index for the outermost Hit
  vector<double> x;
  vector<double> y;
  vector<double> d; //distance of the Hit from the origin

  x.resize(m_nHits);
  y.resize(m_nHits);
  d.resize(m_nHits);

  //search for the index of the Hits with the minimum/maximum distance from the origin
  for (int i = 0; i < m_nHits; i++) {
    x[i] = m_TrackHits[i].getWirePosX();
    y[i] = m_TrackHits[i].getWirePosY();
    d[i] = sqrt(x[i] * x[i] + y[i] * y[i]);

    if (d[i] < d[min]) {
      min = i;
    }
    if (d[i] > d[max]) {
      max = i;
    }
  }
  //assign the correct Hits
  m_innerMostHit = m_TrackHits.at(min);
  m_outerMostHit = m_TrackHits.at(max);

  //Calculate the direction of the Segment in the conformal plane from the positions of the innerMost/outerMost Hits
  double innerX = m_innerMostHit.getConformalX();
  double innerY = m_innerMostHit.getConformalY();
  double outerX = m_outerMostHit.getConformalX();
  double outerY = m_outerMostHit.getConformalY();

//Calculate alternative/better reference point if there are more than one hit in the innermost layer
  /*int minLayer = 999;
  int layerId;
  double x_ref = 0;
  double y_ref = 0;
  int count = 0;
  //find out the innermost layer
    for (int i = 0; i < m_nHits; i++) {
         layerId = m_TrackHits.at(i).getLayerId();
      if (layerId<minLayer) {
        minLayer = layerId;
      }
    }
  //find out if there are more than one hit in the innermost layer and calculate average position
  for (int i =0; i< m_nHits; i++){
  if (m_TrackHits.at(i).getLayerId()==layerId){
    count++;
    x_ref = (x_ref + m_TrackHits.at(i).getConformalX())/count;
    y_ref = (y_ref + m_TrackHits.at(i).getConformalY())/count;

    }
  }*/


//Calculate the average position of all Hits
  /*   double x_ave = 0;
     double y_ave = 0;
     double z_ave = 0;

    for (int i=0; i<m_nHits; i++){
    x_ave = x_ave + m_TrackHits.at(i).getConformalX();
    y_ave = y_ave + m_TrackHits.at(i).getConformalY();
    }
    x_ave = x_ave/m_nHits;
    y_ave = y_ave/m_nHits;
   */


  m_direction.SetX(innerX - outerX);
  m_direction.SetY(innerY - outerY);
  m_direction.SetZ(0);  //conformal plane

//double norm = m_direction.Mag();

}

int CDCSegment::getWireIdDiff()
{
  int minWireId = 1000;
  int maxWireId = 0;
  //Number of wires per layer, hardcoded for now, has to come from the cdc database later
  int NWires[10];
  NWires[0] = 0;
  NWires[1] = 160;
  NWires[2] = 160;
  NWires[3] = 192;
  NWires[4] = 224;
  NWires[5] = 256;
  NWires[6] = 288;
  NWires[7] = 320;
  NWires[8] = 352;
  NWires[9] = 384;

  //Loop over all hits searching for minimal and maximal WireId
  for (int i = 0; i < m_nHits; i++) {
    if (m_TrackHits.at(i).getWireId() < minWireId) minWireId = m_TrackHits.at(i).getWireId();
    if (m_TrackHits.at(i).getWireId() > maxWireId) maxWireId = m_TrackHits.at(i).getWireId();
  }

  //if the segment is 'crossing' WireId = 0, some recalculation is needed to get the correct difference
  if (minWireId < 10 && maxWireId > (NWires[m_superlayerId] - 10)) {
    minWireId = 1000;
    maxWireId = 0;

    for (int i = 0; i < m_nHits; i++) { //loop again over all hits
      //if there is a WireId below 10, shift the values by the total number of wires in this layer
      if (m_TrackHits.at(i).getWireId() < 10) {

        if ((m_TrackHits.at(i).getWireId() + NWires[m_superlayerId]) < minWireId) minWireId = m_TrackHits.at(i).getWireId() + NWires[m_superlayerId];
        if ((m_TrackHits.at(i).getWireId() + NWires[m_superlayerId]) > maxWireId) maxWireId = m_TrackHits.at(i).getWireId() + NWires[m_superlayerId];
      } else {

        if (m_TrackHits.at(i).getWireId() < minWireId) minWireId = m_TrackHits.at(i).getWireId();
        if (m_TrackHits.at(i).getWireId() > maxWireId) maxWireId = m_TrackHits.at(i).getWireId();
      }
    }//end loop over all hits

  }// endif the segment is 'crossing' WireId = 0

  return abs(maxWireId - minWireId);
}


void CDCSegment::clearTrackCandId()
{
  m_trackCandId.erase(m_trackCandId.begin(), m_trackCandId.end());
}




int CDCSegment::shiftAlongZ(TVector3 trackDirection, CDCTrackHit trackHit)
{
  TVector3 StereoHitPos; //conformal position of one point of the segment (outermost hit)
  TVector3 TrackHitPos;  //conformal position of the given TrackHit (starting point for the "track straight line")
  TVector3 perpPoint;  //intersection from "track line" with a to it perpendicular line through segment point
  TVector3 perpDir;    //direction perpendicular to track direction

  double distance;   //distance between the segment point und the intersection point ( =shortest distance from segment to "track line")
  double distanceMax = 10; //start value for the search

  //create a vector with 10 aquidistant values from 0 to 1 to parametrise the wire vector
  double parameter[101];
  for (int i = 0 ; i < 101; i++) {
    parameter[i] = i * 0.01;
  }

  int bestIndex = 0; //index to indicate, which value from the parameter vector is the best

  double posX_o; //position of the hit on the wire
  double posY_o;
  double posZ_o;

  double confX_o; //position of the hit on the wire in the conformal plane
  double confY_o;

  for (int i = 0; i < 101; i++) { //loop over the parameter vector ( = loop over the lenght of the wire)

    //new point along the wire
    posX_o = m_outerMostHit.getWirePosX_f() + parameter[i] * m_outerMostHit.getWireVector().x();
    posY_o = m_outerMostHit.getWirePosY_f() + parameter[i] * m_outerMostHit.getWireVector().y();
    posZ_o = m_outerMostHit.getWirePosZ_f() + parameter[i] * m_outerMostHit.getWireVector().z();

    confX_o = 2 * posX_o / (posX_o * posX_o + posY_o * posY_o);
    confY_o = 2 * posY_o / (posX_o * posX_o + posY_o * posY_o);

    //calculation of the shortest distance between the segment point and the track in the conformal plane
    StereoHitPos.SetX(confX_o);
    StereoHitPos.SetY(confY_o);
    StereoHitPos.SetZ(0);

    TrackHitPos.SetX(trackHit.getConformalX());
    TrackHitPos.SetY(trackHit.getConformalY());
    TrackHitPos.SetZ(0);

    perpDir.SetX(1);
    perpDir.SetY(-trackDirection.x() / trackDirection.y());
    perpDir.SetZ(0);

    double faktor = ((StereoHitPos.y() - TrackHitPos.y()) * trackDirection.x() + (TrackHitPos.x() - StereoHitPos.x()) * trackDirection.y()) / (trackDirection.y() * (perpDir.x() + perpDir.y()));

    perpPoint.SetX(StereoHitPos.x() + faktor*perpDir.x());
    perpPoint.SetY(StereoHitPos.y() + faktor*perpDir.y());
    perpPoint.SetZ(0);

    //finally the result
    distance = (StereoHitPos - perpPoint).Mag();

    //search for the wire point which gives the shortest distance
    if (distance < distanceMax) {
      distanceMax = distance;
      bestIndex = i;
    }
  }

//assign the new better wire points as hit positions
  for (int i = 0; i < m_nHits; i++) {  //loop over all Hits

    TVector3 newPosition;
    double x = m_TrackHits[i].getWirePosX_f() +  parameter[bestIndex] * m_TrackHits[i].getWireVector().x();
    double y = m_TrackHits[i].getWirePosY_f() +  parameter[bestIndex] * m_TrackHits[i].getWireVector().y();
    double z = m_TrackHits[i].getWirePosZ_f() +  parameter[bestIndex] * m_TrackHits[i].getWireVector().z();
    double cx = 2 * x / (x * x + y * y);
    double cy = 2 * y / (x * x + y * y);

    newPosition.SetX(x);
    newPosition.SetY(y);
    newPosition.SetZ(z);

//sets new position
    m_TrackHits[i].setStereoPosition(newPosition);
    m_TrackHits[i].setConformalPosition(cx, cy);

  }
  return bestIndex;
}


void CDCSegment::shiftAlongZ(int index)
{

//create a vector with 10 aquidistant values from 0 to 1 to parametrise the wire vector
  double parameter[101];
  for (int i = 0 ; i < 101; i++) {
    parameter[i] = i * 0.01;
  }

//assign the new better wire points as hit positions
  for (int i = 0; i < m_nHits; i++) {  //loop over all hits

    TVector3 newPosition;
    double x = m_TrackHits[i].getWirePosX_f() +  parameter[index] * m_TrackHits[i].getWireVector().x();
    double y = m_TrackHits[i].getWirePosY_f() +  parameter[index] * m_TrackHits[i].getWireVector().y();
    double z = m_TrackHits[i].getWirePosZ_f() +  parameter[index] * m_TrackHits[i].getWireVector().z();
    double cx = 2 * x / (x * x + y * y);
    double cy = 2 * y / (x * x + y * y);

    newPosition.SetX(x);
    newPosition.SetY(y);
    newPosition.SetZ(z);

//sets new position
    m_TrackHits[i].setStereoPosition(newPosition);
    m_TrackHits[i].setConformalPosition(cx, cy);
  }

}






