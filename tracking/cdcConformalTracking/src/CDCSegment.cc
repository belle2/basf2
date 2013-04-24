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

#include <cdc/geometry/CDCGeometryPar.h>
#include <cmath>
#include <cstdlib>


using namespace std;
using namespace Belle2;
using namespace CDC;

ClassImp(CDCSegment)

CDCSegment::CDCSegment()
{
  m_superlayerId = 0;
  m_Id = 0;
  m_isAxial = 0;
  m_nHits = 0;
  m_direction.SetX(0);
  m_direction.SetY(0);
  m_direction.SetZ(0);
}

CDCSegment::~CDCSegment()
{
}

CDCSegment::CDCSegment(int superlayerId, int Id)
{
  m_superlayerId = superlayerId;
  m_Id = Id;

  if (m_superlayerId % 2 == 0) {
    m_isAxial = true;
  } else {
    m_isAxial = false;
  }

//Set some start values
  m_nHits = 0;

  m_direction.SetX(0);
  m_direction.SetY(0);
  m_direction.SetZ(0);
}

void CDCSegment::addTrackHit(CDCTrackHit& aTrackHit)
{
  m_TrackHits.push_back(aTrackHit);
  update();
}

void CDCSegment::setTrackCandId(const int trackId)
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

void CDCSegment::setTrackCandId(const vector<int> trackId)
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

}

int CDCSegment::getWireIdDiff() const
{

  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();


  int minWireId = 1000;
  int maxWireId = 0;
  int layerId = 0;

  //Loop over all hits searching for minimal and maximal WireId
  for (int i = 0; i < m_nHits; i++) {
    if (m_TrackHits.at(i).getWireId() < minWireId) {
      minWireId = m_TrackHits.at(i).getWireId();
      layerId = m_TrackHits.at(i).getLayerId();
    }
    if (m_TrackHits.at(i).getWireId() > maxWireId)
      maxWireId = m_TrackHits.at(i).getWireId();
  }
  //calculate wireId difference
  unsigned int wireIdDifference = abs(maxWireId - minWireId);

  //for the case that the wireIdDifference is 'crossing 0' (e.g. first WireId 1 and second WireId 300, the difference is not 299), some modification is needed
  if (wireIdDifference > cdcg.nWiresInLayer(layerId) / 2) {
    wireIdDifference = cdcg.nWiresInLayer(layerId) - wireIdDifference;
  }

  return wireIdDifference;
}




float CDCSegment::getCenterPosR() const
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

float CDCSegment::getCenterPosZ() const
{

  float average = 0;
  for (int i = 0; i < m_nHits; i++) { //loop over all hits and sum their z position
    float z = m_TrackHits.at(i).getWirePosition().z();

    average = average + z;
  }
  average = average / m_nHits; //divide the sum by the total number of hits to get the average

  return average;

}





