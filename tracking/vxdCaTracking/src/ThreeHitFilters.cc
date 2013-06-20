/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/ThreeHitFilters.h"
#include <boost/math/special_functions/fpclassify.hpp>
#include <TMathBase.h>
#include <math.h>
#include <boost/math/special_functions/sign.hpp>

using namespace std;
using namespace Belle2;
using boost::math::sign;


double ThreeHitFilters::fullAngle3D()
{
  double angle = acos(m_vecAB.Dot(m_vecBC) / (m_vecAB.Mag() * m_vecBC.Mag())); // 0-pi
  angle = (angle * (180. / M_PI));
  return m_twoHitFilter.filterNan(angle);
} // return unit: ° (0 - 180°)


double ThreeHitFilters::fullAngleXY()
{
  double angle = fullAngle2D(m_vecAB, m_vecBC); // 0-pi
  angle = (angle * (180. / M_PI));
  return m_twoHitFilter.filterNan(angle);
} // return unit: ° (0 - 180°)


double ThreeHitFilters::fullAngleRZ()
{
  TVector3 rzVecAB(m_vecAB.Perp(), m_vecAB[2], 0.);
  TVector3 rzVecBC(m_vecBC.Perp(), m_vecBC[2], 0.);
  double angle = fullAngle2D(rzVecAB, rzVecBC); // 0-pi
  angle = (angle * (180. / M_PI));
  return m_twoHitFilter.filterNan(angle);
} // return unit: ° (0 - 180°)


double ThreeHitFilters::calcCircleDist2IP()
{
  if (m_circleCenterCalculated == false) {
    calcCircleCenter(m_hitA, m_hitB, m_hitC, m_centerABC);
    m_circleCenterCalculated = true;
  }
  if (m_radiusCalculated == false) {
    m_radius = calcRadius(m_hitA, m_hitB, m_hitC, m_centerABC);
    m_radiusCalculated = true;
  }
  return (abs(m_centerABC.Perp() - m_radius)); // distance of closest approach of circle to the IP
} // return unit: cm



double ThreeHitFilters::fullAngle2D(TVector3& vecA, TVector3& vecB)
{
  double angle = acos((vecA[0] * vecB[0] + vecA[1] * vecB[1]) / (sqrt(vecA.Perp() * vecB.Perp())));
  return m_twoHitFilter.filterNan(angle);
}

double ThreeHitFilters::calcRadius(TVector3& a, TVector3& b, TVector3& c, TVector3& circleCenter)
{
  return (sqrt((circleCenter - a).Perp2() + (circleCenter - b).Perp2() + (circleCenter - c).Perp2())) * (1. / 3.); // = radius in [cm], sign here not needed. normally: signKappaAB/normAB1
} // used by calcPt() and calcCircleDist2IP()

int ThreeHitFilters::calcSign(TVector3& a, TVector3& b, TVector3& c)
{
  TVector3 ba = a - b; ba.SetZ(0.);
  TVector3 bc = b - c; bc.SetZ(0.);
  TVector3 nbc = bc.Orthogonal(); //normal vector of m_vecBC
  return sign(nbc * ba);
}
