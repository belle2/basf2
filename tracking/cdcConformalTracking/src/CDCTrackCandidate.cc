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
#include<list>

using namespace std;
using namespace Belle2;

ClassImp(CDCTrackCandidate)

CDCTrackCandidate::CDCTrackCandidate()
{
  m_Id = 0;
  m_nSegments = 0;
  m_nHits = 0;
  m_direction.SetX(0);
  m_direction.SetY(0);
  m_direction.SetZ(0);
  m_chi2 = 0;
  m_pt = 0;
  m_momentum.SetX(0);
  m_momentum.SetY(0);
  m_momentum.SetZ(0);
  m_chargeSign = 0;
}

CDCTrackCandidate::CDCTrackCandidate(const int Id)
{
  m_Id = Id;

  //some start values
  m_nSegments = 0;
  m_nHits = 0;
  m_direction.SetX(0);
  m_direction.SetY(0);
  m_direction.SetZ(0);
  m_chi2 = 0;
  m_pt = 0;
  m_momentum.SetX(0);
  m_momentum.SetY(0);
  m_momentum.SetZ(0);
  m_chargeSign = 0;

}

CDCTrackCandidate::CDCTrackCandidate(CDCTrackCandidate& candidate, const int Id)
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
  m_pt = candidate.getPt();
  m_momentum = candidate.getMomentum();
  m_chargeSign = candidate.getChargeSign();

}

CDCTrackCandidate::~CDCTrackCandidate()
{
}

void CDCTrackCandidate::addSegment(CDCSegment& aSegment)
{
  m_Segments.push_back(aSegment);
  for (int i = 0; i < aSegment.getNHits(); i++) {
    m_TrackHits.push_back(aSegment.getTrackHits().at(i));
  }
  update();
}

void CDCTrackCandidate::removeSegment(const int Id)
{
  list <CDCSegment> segmentList;

  for (int i = 0; i < m_nSegments; i++) {
    segmentList.push_back(m_Segments.at(i));
  }

  list<CDCSegment>::iterator it = segmentList.begin();
  bool doneSeg = false;
  while (it != segmentList.end() && doneSeg == false) {
    if ((*it).getId() == Id) {
      segmentList.erase(it);
      doneSeg = true;
    } else ++it;
  }

  m_Segments.clear();
  for (list<CDCSegment>::iterator it = segmentList.begin(); it != segmentList.end(); ++it) {
    m_Segments.push_back(*it);
  }
  update();
}

void CDCTrackCandidate::setChiSquare(const double chi2)
{
  m_chi2 = chi2;
}

void CDCTrackCandidate::setPt(const double momentum)
{
  m_pt = momentum;
}

void CDCTrackCandidate::setId(const int Id)
{
  m_Id = Id;
}


void CDCTrackCandidate::update()
{
  m_nSegments = m_Segments.size();
  m_nHits = m_TrackHits.size();
  //int nHits = 0;
  //Calculate the direction from the directions of axial segments
  for (int i = 0; i < m_nSegments; i++) {
    if (m_Segments.at(i).getIsAxial() == true) {
      m_direction = m_direction + m_Segments.at(i).getDirection();
    }
    //nHits = nHits+ m_Segments.at(i).getNHits();
  }
  //m_nHits = nHits;
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
  //this method is still not quite finished as there are several possibilities to estimate the momentum
  //the original goal was to have a simple estimation which it good enough for the fit

  //the innermost *stereo* segment
  int minStereoSL = 10;
  int minStereoIndex = 0;

  //the second innermost *stereo* segment
  int secondMinStereoSL = 10;
  int secondMinStereoIndex = 0;

  //the innermost *axial* segment
  int minAxialSL = 10;
  int minAxialIndex = 0;

  for (int i = 0; i < m_nSegments; i++) {
    if (m_Segments.at(i).getSuperlayerId() < minStereoSL && m_Segments.at(i).getIsAxial() == false) {  //innermost stereo segment
      minStereoSL = m_Segments.at(i).getSuperlayerId() ;
      minStereoIndex = i;
    }
    if (m_Segments.at(i).getSuperlayerId() < minAxialSL && m_Segments.at(i).getIsAxial() == true) { //innermost axial segment
      minAxialSL = m_Segments.at(i).getSuperlayerId() ;
      minAxialIndex = i;
    }
  }
  for (int i = 0; i < m_nSegments; i++) {
    if (m_Segments.at(i).getSuperlayerId() < secondMinStereoSL && m_Segments.at(i).getSuperlayerId() > minStereoSL && m_Segments.at(i).getIsAxial() == false) {  //second innermost stereo layer
      secondMinStereoSL = m_Segments.at(i).getSuperlayerId() ;
      secondMinStereoIndex = i;
    }
  }


  //coordinates of the innermost stereo hit
  double x_stIn = m_Segments.at(minStereoIndex).getInnerMostHit().getWirePosition().x();
  double y_stIn = m_Segments.at(minStereoIndex).getInnerMostHit().getWirePosition().y();
  double z_stIn = m_Segments.at(minStereoIndex).getInnerMostHit().getWirePosition().z();

  //coordinates of the outermost hit in the innermost stereo segment
  double x_stOut = m_Segments.at(minStereoIndex).getOuterMostHit().getWirePosition().x();
  double y_stOut = m_Segments.at(minStereoIndex).getOuterMostHit().getWirePosition().y();
  double z_stOut = m_Segments.at(minStereoIndex).getOuterMostHit().getWirePosition().z();

  //coordinates of the innermost hit in the second innermost stereo segment
  double x_st2In = m_Segments.at(secondMinStereoIndex).getInnerMostHit().getWirePosition().x();
  double y_st2In = m_Segments.at(secondMinStereoIndex).getInnerMostHit().getWirePosition().y();
  double z_st2In = m_Segments.at(secondMinStereoIndex).getInnerMostHit().getWirePosition().z();

  //coordinates of the outermost hit in the second innermost stereo segment
  double x_st2Out = m_Segments.at(minStereoIndex).getOuterMostHit().getWirePosition().x();
  double y_st2Out = m_Segments.at(minStereoIndex).getOuterMostHit().getWirePosition().y();
  double z_st2Out = m_Segments.at(minStereoIndex).getOuterMostHit().getWirePosition().z();

  //Px and Py are estimated from the innermost of the innermost axial segment
  //the absolute values are weighted with the pt estimated from the whole track in the AxialTrackFinder
  double ptNorm = sqrt((m_Segments.at(minAxialIndex).getInnerMostHit().getWirePosition().x()) * (m_Segments.at(minAxialIndex).getInnerMostHit().getWirePosition().x()) + (m_Segments.at(minAxialIndex).getInnerMostHit().getWirePosition().y()) * (m_Segments.at(minAxialIndex).getInnerMostHit().getWirePosition().y()));

  double momentumX = (m_Segments.at(minAxialIndex).getInnerMostHit().getWirePosition().x()) / ptNorm * m_pt;
  double momentumY = (m_Segments.at(minAxialIndex).getInnerMostHit().getWirePosition().y()) / ptNorm * m_pt;

  double momentumZ;

  //the estimation of the z momentum component needs a more elaborate approach...
  //at the moment it seems that it make a difference using the innermost stereo segment or the second innermost
  //so this part is still under developement...

  double tempz;
  double tempx;
  double tempy;
  //double znorm;
  double norm;

  if (m_chargeSign >= 0) {
    tempz = (z_st2In + z_st2Out) / 2;
    tempx = (x_st2In + x_st2Out) / 2;
    tempy = (y_st2In + y_st2Out) / 2;
    //znorm = sqrt(tempx * tempx + tempy * tempy);
    norm = sqrt(tempx * tempx + tempy * tempy + tempz * tempz);
    momentumZ = tempz / norm;
  } else {
    tempz = (z_stIn + z_stOut) / 2;
    tempx = (x_stIn + x_stOut) / 2;
    tempy = (y_stIn + y_stOut) / 2;
    //znorm = sqrt(tempx * tempx + tempy * tempy);
    norm = sqrt(tempx * tempx + tempy * tempy + tempz * tempz);

    momentumZ = tempz / norm;
  }

  m_momentum.SetX(momentumX);
  m_momentum.SetY(momentumY);
  m_momentum.SetZ(momentumZ);

  B2DEBUG(150, "Estimated momentum: " << momentumX << "  " << momentumY << "  " << momentumZ);


}

void CDCTrackCandidate::setChargeSign(const int sign)
{
  m_chargeSign = sign;
}

