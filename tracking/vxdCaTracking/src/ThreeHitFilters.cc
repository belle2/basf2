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

/// TODO: evtly do a 'nan'-check for return values


double ThreeHitFilters::calcAngle3D()
{
  return ((m_x2 + m_y2 + m_z2) / (m_vecAB.Mag2() * m_vecBC.Mag2())); // fullCalc would be acos(m_vecAB.Dot(m_vecBC) / m_vecAB.Mag()*m_vecBC.Mag())
} // return unit: none (calculation for degrees is incomplete, if you want readable numbers, use fullAngle3D instead)


double ThreeHitFilters::fullAngle3D()
{
  double angle = acos(m_vecAB.Dot(m_vecBC) / m_vecAB.Mag() * m_vecBC.Mag()); // 0-pi
  return (angle * (180. / M_PI));
} // return unit: ° (0 - 180°)


double ThreeHitFilters::calcAngleXY()
{
  return ((m_x2 + m_y2) / (m_vecAB.Perp2() * m_vecBC.Perp2())); // fullAngle:
} // return unit: none (calculation for degrees is incomplete, if you want readable numbers, use fullAngleXY instead)


double ThreeHitFilters::fullAngleXY()
{
  double angle = fullAngle2D(m_vecAB, m_vecBC); // 0-pi
  return (angle * (180. / M_PI));
} // return unit: ° (0 - 180°)


double ThreeHitFilters::calcAngleRZ()
{
  TVector3 rzVecAB(m_vecAB.Perp(), m_vecAB[2], 0.);
  TVector3 rzVecBC(m_vecBC.Perp(), m_vecBC[2], 0.);
  return calcAngle2D(rzVecAB, rzVecBC);
} // return unit: none (calculation for degrees is incomplete, if you want readable numbers, use fullAngleRZ instead)


double ThreeHitFilters::fullAngleRZ()
{
  TVector3 rzVecAB(m_vecAB.Perp(), m_vecAB[2], 0.);
  TVector3 rzVecBC(m_vecBC.Perp(), m_vecBC[2], 0.);
  double angle = fullAngle2D(rzVecAB, rzVecBC); // 0-pi
  return (angle * (180. / M_PI));
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


double ThreeHitFilters::calcPt()
{
  if (m_circleCenterCalculated == false) {
    calcCircleCenter(m_hitA, m_hitB, m_hitC, m_centerABC);
    m_circleCenterCalculated = true;
  }
  if (m_radiusCalculated == false) {
    m_radius = calcRadius(m_hitA, m_hitB, m_hitC, m_centerABC);
    m_radiusCalculated = true;
  }
  return (0.0045 * m_radius); // pT[GeV/c] = 0.3*B[T]*r[m] = 0.45*r[cm]/100 = 0.0045*r[cm]
} // return unit: GeV/c


double ThreeHitFilters::calcDeltaSlopeRZ()
{
  m_twoHitFilter.resetValues(m_hitA, m_hitB);
  double slopeAB = m_twoHitFilter.calcSlopeRZ();
  m_twoHitFilter.resetValues(m_hitB, m_hitC);
  double slopeBC = m_twoHitFilter.calcSlopeRZ();

  return (slopeBC / slopeAB); // value should be near 1
} // return unit: none



double ThreeHitFilters::calcHelixFit()
{
  if (m_circleCenterCalculated == false) { calcCircleCenter(m_hitA, m_hitB, m_hitC, m_centerABC); }
  TVector3 points2hitA = m_hitA - m_centerABC;
  TVector3 points2hitB = m_hitB - m_centerABC;
  TVector3 points2hitC = m_hitC - m_centerABC;
  double alfaAB = calcAngle2D(points2hitA, points2hitB);
  double alfaBC = calcAngle2D(points2hitB, points2hitC);
  // real calculation: ratio is (m_vecij[2] = deltaZ): alfaAB/deltaZab : alfaBC/deltaZbc, the following equation saves two times '/'
  return (alfaAB * m_vecBC[2]) / (alfaBC * m_vecAB[2]) ;
} // return unit: none



double ThreeHitFilters::calcAngle2D(TVector3& vecA, TVector3& vecB)
{
  return ((vecA[0] * vecB[0] + vecA[1] * vecB[1]) / (vecA.Perp2() * vecB.Perp2()));
}



double ThreeHitFilters::fullAngle2D(TVector3& vecA, TVector3& vecB)
{
  return acos((vecA[0] * vecB[0] + vecA[1] * vecB[1]) / (sqrt(vecA.Perp() * vecB.Perp())));
}



double ThreeHitFilters::calcRadius(TVector3& a, TVector3& b, TVector3& c, TVector3& circleCenter)
{
  return (sqrt((circleCenter - a).Perp2() + (circleCenter - b).Perp2() + (circleCenter - c).Perp2())) * (1. / 3.); // = radius in [cm], sign here not needed. normally: signKappaAB/normAB1
} // used by calcPt() and calcCircleDist2IP()



void ThreeHitFilters::calcCircleCenter(TVector3& a, TVector3& b, TVector3& c, TVector3& circleCenter)
{
  TVector3 b2c = b - c;
  TVector3 a2b = a - b;
  TVector3 cAB = 0.5 * a2b + b; //([kx ky]  -[jx jy])/2 + [jx jy] = central point of outer segment (k-j)/2+j
  TVector3 cBC = 0.5 * b2c + c; // = central point of inner segment (l-k)/2+k
  TVector3 nAB(-a2b(1), a2b(0), 0.); //normal vector of m_vecAB
  TVector3 nBC(-b2c(1), b2c(0), 0.); //normal vector of m_vecBC
  double muVal = (((cAB(1) - cBC(1)) / nBC(1)) + (((cBC(0) - cAB(0)) / nAB(0)) * nAB(1) / nBC(1))) / (1. - ((nBC(0) / nAB(0)) * (nAB(1) / nBC(1))));
  circleCenter.SetX(cBC(0) + muVal * nBC(0)); // x-coord of intersection point
  circleCenter.SetY(cBC(1) + muVal * nBC(1)); // y-coord of intersection point
  circleCenter.SetZ(0.);
}



int ThreeHitFilters::calcSign(TVector3& a, TVector3& b, TVector3& c)
{
  TVector3 ba = a - b; ba.SetZ(0.);
  TVector3 bc = b - c; bc.SetZ(0.);
  TVector3 nbc = bc.Orthogonal(); //normal vector of m_vecBC
  return sign(nbc * ba);
}
