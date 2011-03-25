/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCTrackCandidate.h"
#include "tracking/cdcConformalTracking/AxialTrackFinder.h"
#include <cmath>

using namespace std;
using namespace Belle2;

ClassImp(CDCTrackCandidate)

CDCTrackCandidate::CDCTrackCandidate()
{
}

CDCTrackCandidate::CDCTrackCandidate(int Id)
{
  m_Id = Id;

  //some start values
  m_nSegments = 0;
  m_nHits = 0;
  m_direction.SetX(0);
  m_direction.SetY(0);
  m_direction.SetZ(0);
  m_chi2 = 0;
  m_momentumValue = 0;
  m_chargeSign = 0;

}

CDCTrackCandidate::CDCTrackCandidate(CDCTrackCandidate &candidate, int Id)
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
  m_chargeSign = candidate.getChargeSign();

  m_mcParticles = candidate.getMCParticles();

}

CDCTrackCandidate::~CDCTrackCandidate()
{
}

void CDCTrackCandidate::addSegment(CDCSegment & aSegment)
{
  m_Segments.push_back(aSegment);
  for (int i = 0; i < aSegment.getNHits(); i++) {
    m_TrackHits.push_back(aSegment.getTrackHits().at(i));
  }
  update();
}

void CDCTrackCandidate::removeSegment(int Id)
{

  for (int i = 0; i < m_nSegments; i++) {
    if (m_Segments.at(i).getId() == Id) {
      m_Segments.erase(m_Segments.begin() + i);
      //B2INFO("Remove segment from candidate!");
      update();
    }
  }
}

void CDCTrackCandidate::setChiSquare(double chi2)
{
  m_chi2 = chi2;
}

void CDCTrackCandidate::setMomentumValue(double momentum)
{
  m_momentumValue = momentum;
}


void CDCTrackCandidate::update()
{
  m_nSegments = m_Segments.size();
  m_nHits = m_TrackHits.size();

//Calculate the direction from the directions of axial segments
  for (int i = 0; i < m_nSegments; i++) {
    if (m_Segments.at(i).getIsAxial() == true) {
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
    if (m_TrackHits.at(i).getSuperlayerId() > max && m_TrackHits.at(i).getIsAxial() == true) {
      max = m_TrackHits.at(i).getSuperlayerId() ;
      max_index = i;
    }
    if (m_TrackHits.at(i).getSuperlayerId() < min && m_TrackHits.at(i).getIsAxial() == true) {
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
    if (m_Segments.at(i).getSuperlayerId() > maxSL && m_Segments.at(i).getIsAxial() == true) {
      maxSL = m_Segments.at(i).getSuperlayerId() ;
      max_indexSeg = i;
    }
    if (m_Segments.at(i).getSuperlayerId() < minSL && m_Segments.at(i).getIsAxial() == true) {
      minSL = m_Segments.at(i).getSuperlayerId() ;
      min_indexSeg = i;
    }
  }
  m_innerMostSegment = m_Segments.at(min_indexSeg);
  m_outerMostSegment = m_Segments.at(max_indexSeg);

}

void CDCTrackCandidate::estimateMomentum()
{

  //Find the innermost *stereo* segment
  int minSL = 10;
  int min_index = 0;

  for (int i = 0; i < m_nSegments; i++) {
    if (m_Segments.at(i).getSuperlayerId() < minSL && m_Segments.at(i).getIsAxial() == false) {
      minSL = m_Segments.at(i).getSuperlayerId() ;
      min_index = i;
    }
  }
  //The coordinates of the innermost stereo hit build the momentum vector (0,0,0 supposed to be the starting point of the track)
  double x = m_Segments.at(min_index).getInnerMostHit().getWirePosition().x();
  double y = m_Segments.at(min_index).getInnerMostHit().getWirePosition().y();
  double z = m_Segments.at(min_index).getInnerMostHit().getWirePosition().z();
  double norm = sqrt(x * x + y * y + z * z);

  m_momentumVector.SetX((x / norm)*m_momentumValue);
  m_momentumVector.SetY((y / norm)*m_momentumValue);
  m_momentumVector.SetZ((z / norm)*m_momentumValue);
}

void CDCTrackCandidate::setChargeSign(int sign)
{

  m_chargeSign = sign;

}

void CDCTrackCandidate::addMCParticle(int Id)
{

  bool already = false;
  for (unsigned int i = 0; i < m_mcParticles.size(); i++) { //loop over mcParticles which are already there

    if (m_mcParticles.at(i).first == Id) { //if there is already an entry for this mcParticleId
      m_mcParticles.at(i).second = m_mcParticles.at(i).second + 1;  //increase the number of hits from this mcParticle by 1
      already = true;
    }
  }// end loop over mcParticles

  if (already == false) {                 //if this is the first entry for thi mcParticleId, create new entry
    pair <int, int> newEntry(Id, 1);
    m_mcParticles.push_back(newEntry);
  }
}

void CDCTrackCandidate::evaluateMC()
{

  double max = 0;     //variable to mark the highest number of hits
  int indexMax = 0;   //variable to mart the index of the mcParticle which contributed the highest number of hits

  //Search for maximum
  for (unsigned int i = 0; i < m_mcParticles.size(); i++) {
    if (m_mcParticles.at(i).second > max) {
      max = m_mcParticles.at(i).second;
      indexMax = i;
    }
  }

  //Assign results
  double fraction = double(max) / double(m_nHits) * 100;
  m_purity = fraction;
  m_mcId = m_mcParticles.at(indexMax).first;

}

double CDCTrackCandidate::getRatioForMCP(int mcId)
{

  double nHits = 0;

  //Search for number of Hits contributed by this mcParticleId
  for (unsigned int i = 0; i < m_mcParticles.size(); i++) {
    if (m_mcParticles.at(i).first == mcId) {
      nHits = m_mcParticles.at(i).second;
    }
  }

  double fraction = double(nHits) / double(m_nHits) * 100;

  return fraction;

}







