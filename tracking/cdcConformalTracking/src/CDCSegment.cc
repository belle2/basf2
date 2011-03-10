/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCSegment.h"

#include <tracking/cdcConformalTracking/AxialTrackFinder.h>

#include <cmath>
#include <cstdlib>


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

  //-------------------------------
  //m_chi2 = 0;
  //m_tempCellState = 1;
  //m_cellState = 1;
  //-------------------------------

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

// for (int i = 0; i < m_nHits; i++) {
//   m_TrackHits[i].setIsUsed(isUsed);
// }
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

void CDCSegment::clearTrackCandId()
{

  m_trackCandId.erase(m_trackCandId.begin(), m_trackCandId.end());
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
    x[i] = m_TrackHits[i].getWirePosition().x();
    y[i] = m_TrackHits[i].getWirePosition().y();
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




float CDCSegment::getCenterPosR()
{

  float average = 0;
  for (int i = 0; i < m_nHits; i++) { //loop over all hits and sum their distance from the origin in the r-phi plane
    float x = m_TrackHits.at(i).getWirePosition().x();
    float y = m_TrackHits.at(i).getWirePosition().y();
    average = average + sqrt(x * x + y * y);
  }
  average = average / m_nHits;  //divide the sum by the total number of hits to get the average

  return average;

}

float CDCSegment::getCenterPosZ()
{

  float average = 0;
  for (int i = 0; i < m_nHits; i++) { //loop over all hits and sum their z position
    float z = m_TrackHits.at(i).getWirePosition().z();

    average = average + z;
  }
  average = average / m_nHits; //divide the sum by the total number of hits to get the average

  return average;

}

//Methods for segment fitting
//-----------------------------------------------------
/*void CDCSegment::setChiSquare(double chi2)
{
  m_chi2 = chi2;
}

void CDCSegment::removeTrackHit(int index){
  m_TrackHits.erase(m_TrackHits.begin()+index);
  update();
}*/
//------------------------------------------------------

//Methods for CellularAxialTrackFinder
//------------------------------------------------------
/*
void CDCSegment::setTempCellState(int tempCellState){
  m_tempCellState = tempCellState;
}

void CDCSegment::updateCellState(){
  m_cellState = m_tempCellState;
}
*/
//------------------------------------------------------





