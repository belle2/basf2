/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2021 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Wessel                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <tracking/datcon/optimizedDATCON/filters/twoHitFilters/TwoHitVariables.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/geometry/B2Vector3.>

#include <cmath>

namespace Belle2 {

  /** calculates the angle between the hits/vectors (3D), returning unit: angle in degrees */
  double getAngle3D(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit)
  {
    B2Vector3D outerVector = oHit - cHit;
    B2Vector3D innerVector = cHit - iHit;

    double result = acos(outerVector.Dot(innerVector) / (outerVector.Mag() * innerVector.Mag())); // 0-pi
    result = (result * (180. / M_PI));
    return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
  }


  /** calculates the angle between the hits/vectors (3D), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use Angle3DFull instead) */
  double getAngle3DSimple(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit)
  {
    B2Vector3D outerVector = oHit - cHit;
    B2Vector3D innerVector = cHit - iHit;

    // fullCalc would be acos(m_vecAB.Dot(m_vecBC) / m_vecAB.Mag()*m_vecBC.Mag()), but here time-consuming parts have been neglected
    double result = outerVector.Dot(innerVector) / (outerVector.Mag2() * innerVector.Mag2());
    return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
  }


  /** calculates the angle between the hits/vectors (RZ), returning unit: angle in degrees */
  double getAngleRZ(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit)
  {
    B2Vector3D outerVector = oHit - cHit;
    B2Vector3D innerVector = cHit - iHit;

    B2Vector3D rzVecAB(outerVector.Perp(), outerVector[2], 0.);
    B2Vector3D rzVecBC(innerVector.Perp(), innerVector[2], 0.);

    double result = acos(getCosXY(rzVecAB, rzVecBC));  // 0-pi
    result *= double(180. / M_PI);
    return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
  } // return unit: ° (0 - 180°)


  /** calculates the angle between the hits/vectors (RZ), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleRZFull instead)  */
  double getAngleRZSimple(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit)
  {
    B2Vector3D outerVector = oHit - cHit;
    B2Vector3D innerVector = cHit - iHit;

    B2Vector3D rzVecAB(outerVector.Perp(), outerVector[2], 0.);
    B2Vector3D rzVecBC(innerVector.Perp(), innerVector[2], 0.);

    return getCosXY(rzVecAB, rzVecBC);
  } // return unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleRZFull instead)


  /** calculates the angle between the hits/vectors (XY), returning unit: angle in degrees */
  double getAngleXY(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit)
  {
    B2Vector3D outerVector = oHit - cHit;
    B2Vector3D innerVector = cHit - iHit;

    double result = acos(getCosXY(outerVector, innerVector));   // 0-pi
    result *= double(180. / M_PI); // do conversion into degree
    return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
  } // return unit: ° (0 - 180°)


  /** calculates an estimation of circleCenter position, result is returned as the x and y value of the B2Vector3. */
  B2Vector3D getCircleCenterXY(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit)
  {
    // calculates the intersection point using Cramer's rule.
    // x_1+s*n_1==x_2+t*n_2 --> n_1 *s - n_2 *t == x_2 - x_1 --> http://en.wikipedia.org/wiki/Cramer%27s_rule
    double inX = cHit.X() - iHit.X(); // x value of the normal vector of the inner segment (cHit-iHit)
    double inY = cHit.Y() - iHit.Y(); // y value of the normal vector of the inner segment (cHit-iHit)
    double outX = oHit.X() - cHit.X(); // x value of the normal vector of the outer segment (oHit-cHit)
    double outY = oHit.Y() - cHit.Y(); // y value of the normal vector of the outer segment (oHit-cHit)

    //searching solution for Ax = b, aij are the matrix elements of A, bi are elements of b
    double a11 = inY;
    double a12 = -inX;
    double a21 = -outY;
    double a22 = outX;
    double b1 = cHit.X() + outX * 0.5 - (iHit.X() + inX * 0.5);
    double b2 = cHit.Y() + outY * 0.5 - (iHit.Y() + inY * 0.5);

//       if (a11 * a22 == a12 * a21) { throw Straight_Line(); }
//     if (a11 * a22 == a12 * a21) { return B2Vector3D(1e30, 1e30, 1e30); }
    if (a11 * a22 == a12 * a21) { return NAN; }

    double s = (b1 * a22 - b2 * a21) / (a11 * a22 - a12 * a21); //the determinant is zero if the three hits are on a line in (x,y).

    return B2Vector3D(iHit.X() + inX * 0.5 + s * inY, iHit.Y() + inY * 0.5 - s * inX, 0.);
  }


  /** calculates the distance of the point of closest approach of circle to the IP, returning unit: cm */
  double getCircleDistanceIP(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit)
  {
    B2Vector3D circleCenter = getCircleCenterXY(oHit, cHit, iHit);
    if (std::isnan(circleCenter)) return NAN;
    double circleRadius = calcAvgDistanceXY(oHit, cHit, iHit, circleCenter);

    // distance of closest approach of circle to the IP :
    // WARNING only valid for IP=0,0,X
    return (fabs(circleCenter.Perp() - circleRadius));
  } // return unit: cm


  /** calculates the estimation of the circle radius of the 3-hit-tracklet, returning unit: cm. */
  double getCircleRadius(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit)
  {
    B2Vector3D circleCenter = getCircleCenterXY(oHit, cHit, iHit);
    if (std::isnan(circleCenter)) return NAN;
    return calcAvgDistanceXY(oHit, cHit, iHit, circleCenter);
  } // return unit: cm

  /** helper function with calculates the average distance in XY from the given center
  @param oHit: first point for which the distance is evaluated
  @param cHit: second point for which the distance is evaluated
  @param iHit: third point for which the distance is evaluated
  @param circleCenter: the center from which the relative distance is evaluated
  */
  double calcAvgDistanceXY(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit, const B2Vector3D& circleCenter)
  {
    return (sqrt(std::pow(circleCenter.X() - oHit.X(), 2) + std::pow(circleCenter.Y() - oHit.Y(), 2)) +
            sqrt(std::pow(circleCenter.X() - cHit.X(), 2) + std::pow(circleCenter.Y() - cHit.Y(), 2)) +
            sqrt(std::pow(circleCenter.X() - iHit.X(), 2) + std::pow(circleCenter.Y() - iHit.Y(), 2))) / 3.;
  } // = radius in [cm], sign here not needed. normally: signKappaAB/normAB1


  /** calculates the angle between the hits/vectors (XY),
   * returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleXYFull instead) */
  double getCosAngleXY(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit)
  {
    B2Vector3D outerVector = oHit - cHit;
    B2Vector3D innerVector = cHit - iHit;

    double result = (outerVector.X() * innerVector.X() + outerVector.Y() * innerVector.Y()) / (outerVector.Perp() *
                    innerVector.Perp());

    return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
  } // return unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleXYFull instead)


  /** calculates deviations in the slope of the inner segment and the outer segment, returning unit: none */
  double getDeltaSlopeRZ(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit)
  {
    double slopeOC = getRZSlope(oHit, cHit);
    double slopeCI = getRZSlope(cHit, iHit);

    return slopeCI - slopeOC;
  } // return unit: none


  /** compares the "slopes" z over arc length. calcDeltaSlopeZOverS is invariant under rotations in the r-z plane. */
  double getDeltaSlopeZoverS(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit)
  {
    B2Vector3D circleCenter = getCircleCenterXY(oHit, cHit, iHit);
    if (std::isnan(circleCenter)) return NAN;
    double circleRadius = calcAvgDistanceXY(oHit, cHit, iHit, cCenter);
    B2Vector3D vecOuter2cC  = oHit - circleCenter;
    B2Vector3D vecCenter2cC = cHit - circleCenter;
    B2Vector3D vecInner2cC  = iHit - circleCenter;

    // WARNING: this is only approximately S (valid in the limit of small angles) but might be OK for this use!!!
    //  want to replace id with 2*sin ( alfa ) * circleRadius
    double alfaOCr = acos(getCosXY(vecOuter2cC, vecCenter2cC)) * circleRadius ;
    double alfaCIr = acos(getCosXY(vecCenter2cC, vecInner2cC)) * circleRadius ;

    // Beware of z>r!:
    double result = (asin(double(oHit.Z() - cHit.Z()) / alfaOCr)) - asin(double(cHit.Z() - iHit.Z()) / alfaCIr);

    return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
  }


  /** calculates the helixparameter describing the deviation in arc length per unit in z.
   * returning unit: radians*cm */
  double getDeltaSoverZ(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit)
  {
    B2Vector3D circleCenter = getCircleCenterXY(oHit, cHit, iHit);
    if (std::isnan(circleCenter)) return NAN;
    B2Vector3D vecOuter2cC  = oHit - circleCenter;
    B2Vector3D vecCenter2cC = cHit - circleCenter;
    B2Vector3D vecInner2cC  = iHit - circleCenter;
    double alfaOC = acos(getCosXY(vecOuter2cC, vecCenter2cC));
    double alfaCI = acos(getCosXY(vecCenter2cC, vecInner2cC));

    // equals to alfaAB/dZAB and alfaBC/dZBC, but this solution here can not produce a division by zero:
    return (alfaOC * double(cHit.Z() - iHit.Z())) - (alfaCI * double(oHit.Z() - cHit.Z()));
  } // return unit: radians*cm


// //     /** return the time difference (ns) among the V and U side clusters of th ecenter space point */
// //     double getDeltaTimeVU(const SpacePoint&, const SpacePoint& cHit, const SpacePoint&)
// //     {
// //       return cHit.TimeV() - cHit.TimeU();
// //     }


  /** calculates the helixparameter describing the deviation in z per unit angle, returning unit: none */
  double performHelixParamterFit(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit)
  {
    B2Vector3D circleCenter = getCircleCenterXY(oHit, cHit, iHit);
    if (std::isnan(circleCenter)) return NAN;

    B2Vector3D vecOuter2cC  = oHit - circleCenter;
    B2Vector3D vecCenter2cC = cHit - circleCenter;
    B2Vector3D vecInner2cC  = iHit - circleCenter;

    double alfaAB = getCosXY(vecOuter2cC, vecCenter2cC);
    double alfaBC = getCosXY(vecCenter2cC, vecInner2cC);

    // real calculation: ratio is (m_vecij[2] = deltaZ): alfaAB/deltaZab : alfaBC/deltaZbc, the following equation saves two times '/'
    double result = (alfaAB * double(cHit.Z() - iHit.Z())) / (alfaBC * double(oHit.Z() - cHit.Z()));

    return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
  } // return unit: none


  /** calculates the estimation of the transverse momentum of the 3-hit-tracklet, returning unit: GeV/c */
  double getSimplePTEstimate(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit)
  {
    B2Vector3D circleCenter = getCircleCenterXY(oHit, cHit, iHit);
    if (std::isnan(circleCenter)) return NAN;
    double circleRadius = calcAvgDistanceXY(oHit, cHit, iHit, circleCenter);

    // 0.3 * B * R, but with R in cm instead of m -> (0.3 -> 0.003)
    // 0.00299792458 * 1.5 = 0.00449688687
    return 0.00449688687 * circleRadius;
  } // return unit: GeV/c


  /** calculates calculates the sign of the curvature of given 3-hit-tracklet.
  * a positive value represents a left-oriented curvature, a negative value means having a right-oriented curvature.
  * 0 means that it is exactly straight or that two hits are identical.
  * first vector should be outer hit, second = center hit, third is inner hit. */
  int getCurvatureSign(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit)
  {
    using boost::math::sign;
    B2Vector3D ba(oHit.X() - cHit.X(), oHit.Y() - cHit.Y(), 0.0);
    B2Vector3D bc(cHit.X() - iHit.X(), cHit.Y() - iHit.Y(), 0.0);
    return sign(bc.Orthogonal() * ba); //normal vector of m_vecBC times segment of ba
  }
}
