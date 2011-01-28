/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCTrack.h"
#include <cmath>

using namespace std;
using namespace Belle2;

ClassImp(CDCTrack)

CDCTrack::CDCTrack()
{
}

CDCTrack::CDCTrack(int Id)
{
  m_Id = Id;

  //some start values
  m_nSegments = 0;
  m_nHits = 0;
  m_direction.SetX(0);
  m_direction.SetY(0);
  m_direction.SetZ(0);
  m_chi2 = 0;

}

CDCTrack::CDCTrack(CDCTrack &candidate, int Id)
{

  m_Id = Id;
  m_nSegments = candidate.getNSegments();
  m_nHits = candidate.getNHits();

  m_Segments = candidate.getSegments();
  m_TrackHits = candidate.getTrackHits();

  m_direction = candidate.getDirection();

  m_innerMostHit = candidate.getInnerMostHit();
  m_outerMostHit = candidate.getOuterMostHit();
  m_innerMostSegment = candidate.getInnerMostSegment();
  m_outerMostSegment = candidate.getOuterMostSegment();

  m_chi2 = candidate.getChiSquare();
  m_momentumValue = candidate.getMomentumValue();

}

CDCTrack::~CDCTrack()
{
}

void CDCTrack::addSegment(CDCSegment aSegment)
{
  m_Segments.push_back(aSegment);
  for (int i = 0; i < aSegment.getNHits(); i++) {
    m_TrackHits.push_back(aSegment.getTrackHits().at(i));
  }
  update();
}

void CDCTrack::removeSegment(int Id)
{

  for (int i = 0; i < m_nSegments; i++) {
    if (m_Segments.at(i).getId() == Id) {
      m_Segments.erase(m_Segments.begin() + i);
      //B2INFO("Remove segment from candidate!");
      update();
    }
  }
}

void CDCTrack::addTrackHit(CDCTrackHit aTrackHit)
{
  m_TrackHits.push_back(aTrackHit);
  update();
}

void CDCTrack::setChiSquare(double chi2)
{
  m_chi2 = chi2;
}

void CDCTrack::setMomentumValue(double momentum)
{
  m_momentumValue = momentum;
}


void CDCTrack::update()
{
  m_nSegments = m_Segments.size();
  m_nHits = m_TrackHits.size();

//Calculate the direction from the directions of axial segments
  for (int i = 0; i < m_nSegments; i++) {
    if (m_Segments.at(i).getSuperlayerId() % 2 != 0) {
      m_direction = m_direction + m_Segments.at(i).getDirection();
    }
  }
  //double norm = m_direction.Mag();
  //m_direction.SetX( m_direction.x()/norm);
  //m_direction.SetY( m_direction.y()/norm);
  //m_direction.SetZ( m_direction.z()/norm);

//Assign correct innermost and outermost hits
  int min = 10;
  int max = 0;
  int max_index = 0;
  int min_index = 0;

  for (unsigned i = 0; i < m_TrackHits.size(); i++) {
    if (m_TrackHits.at(i).getSuperlayerId() > max && m_TrackHits.at(i).getSuperlayerId() % 2 != 0) {
      max = m_TrackHits.at(i).getSuperlayerId() ;
      max_index = i;
    }
    if (m_TrackHits.at(i).getSuperlayerId() < min && m_TrackHits.at(i).getSuperlayerId() % 2 != 0) {
      min = m_TrackHits.at(i).getSuperlayerId() ;
      min_index = i;
    }
  }
  m_innerMostHit = m_TrackHits.at(min_index);
  m_outerMostHit = m_TrackHits.at(max_index);

//Assign correct innermost and outermost segments
  int minSL = 10;
  int maxSL = 0;
  int max_indexSeg = 0;
  int min_indexSeg = 0;

  for (int i = 0; i < m_nSegments; i++) {
    if (m_Segments.at(i).getSuperlayerId() > maxSL && m_Segments.at(i).getSuperlayerId() % 2 != 0) {
      maxSL = m_Segments.at(i).getSuperlayerId() ;
      max_indexSeg = i;
    }
    if (m_Segments.at(i).getSuperlayerId() < minSL && m_Segments.at(i).getSuperlayerId() % 2 != 0) {
      minSL = m_Segments.at(i).getSuperlayerId() ;
      min_indexSeg = i;
    }
  }
  m_innerMostSegment = m_Segments.at(min_indexSeg);
  m_outerMostSegment = m_Segments.at(max_indexSeg);

}

void CDCTrack::estimateMomentum()
{
  //Find the innermost *stereo* segment
  int minSL = 10;
  int min_index = 0;

  for (int i = 0; i < m_nSegments; i++) {
    if (m_Segments.at(i).getSuperlayerId() < minSL && m_Segments.at(i).getSuperlayerId() % 2 == 0) {
      minSL = m_Segments.at(i).getSuperlayerId() ;
      min_index = i;
    }
  }
  //The coordinates of the innermost stereo hit build the momentum vector (0,0,0 supposed to be the starting point of the track)
  double x = m_Segments.at(min_index).getInnerMostHit().getWirePosition().x();
  double y = m_Segments.at(min_index).getInnerMostHit().getWirePosition().y();
  double z = m_Segments.at(min_index).getInnerMostHit().getWirePosition().z();
  double norm = sqrt(x * x + y * y + z * z);

  m_momentumVector.SetX(x / norm);
  m_momentumVector.SetY(y / norm);
  m_momentumVector.SetZ(z / norm);

  //B2INFO("Momentum vector: "<<m_momentumVector.X()<<"  "<<m_momentumVector.Y()<<"  "<<m_momentumVector.Z());


}






