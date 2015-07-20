/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <math.h>

#include <tracking/vectorTools/B2Vector3.h>
#include <boost/math/special_functions/sign.hpp>


namespace Belle2 {

  /** Namespace for selectionVariable - helper functions. contains functions needed for several selection-variable-implementations of 2-, 3- and 4-hitfilters. */
  namespace SelVarHelper {
    /** this exception is thrown by the CircleFit and occurs when the track is too straight */
    BELLE2_DEFINE_EXCEPTION(Straight_Line, "The hits are on a straight Line (or indistinguishably near to being on it).");



    /** returns the perpendicular magnitude squared of given pointType */
    template<class PointType, class ReturnType>
    ReturnType calcPerp2(const PointType& aHit)
    {
      return (aHit.X() * aHit.X() + aHit.Y() * aHit.Y());
    }


    /** returns the perpendicular magnitude of given pointType */
    template<class PointType, class ReturnType>
    ReturnType calcPerp(const PointType& aHit) { return sqrt(calcPerp2(aHit)); }


    /** returns the magnitude squared of given pointType */
    template<class PointType, class ReturnType>
    ReturnType calcMag2(const PointType& aHit)
    {
      return (aHit.X() * aHit.X() + aHit.Y() * aHit.Y() + aHit.Z() * aHit.Z());
    }


    /** returns the magnitude of given pointType */
    template<class PointType, class ReturnType>
    ReturnType calcMag(const PointType& aHit) { return sqrt(calcMag2(aHit)); }


    /** returns scalar product of given two pointTypes */
    template<class PointType, class ReturnType>
    ReturnType calcDot(const PointType& aHit, const PointType& bHit)
    {
      return (aHit.X() * bHit.X() + aHit.Y() * bHit.Y() + aHit.Z() * bHit.Z());
    }


    /** returns B2Vector3 containing point a - b */
    template<class PointType, class ReturnType>
    B2Vector3<ReturnType> doAMinusB(const PointType& a, const PointType& b)
    {
      return B2Vector3<ReturnType>(a.X() - b.X(), a.Y() - b.Y(), a.Z() - b.Z());
    }


    /** calculates the angle between the hits/vectors (2D), generalized, returning unit: none (incomplete calculation). */
    template<class PointType, class ReturnType>
    double calcAngle2D(const PointType& vecA, const PointType& vecB)
    {
      double result = ((vecA.X() * vecB.X() + vecA.Y() * vecB.Y()) / sqrt(calcPerp2(vecA) * calcPerp2(vecB)));
      return (std::isnan(result) || std::isinf(result)) ? 0 : result;
    }


    /** calculates the angle between the hits/vectors (2D), generalized, returning unit: angle in radians
     */
    template<class PointType, class ReturnType>
    double fullAngle2D(const PointType& vecA, const PointType& vecB)
    {
      return acos(calcAngle2D(vecA, vecB));
    }


    /** calculates an estimation of the radius of given hits and existing estimation of circleCenter, returning unit: radius in [cm] (positive value)*/
    template<class PointType, class ReturnType>
    double calcRadius(const PointType& a, const PointType& b, const PointType& c, const PointType& circleCenter)
    {
      return (doAMinusB(circleCenter, a).Perp() + doAMinusB(circleCenter, b).Perp() + doAMinusB(circleCenter, c).Perp()) /
             3.;   // = radius in [cm], sign here not needed. normally: signKappaAB/normAB1
    }


    /** calculates an estimation of circleCenter position, result is written into the 4th input-parameter */
    template<class PointType, class DataType>
    B2Vector3<DataType> calcCircleCenter(const PointType& a, const PointType& b, const PointType& c)
    {
      // calculates the intersection point using Cramer's rule.
      // x_1+s*n_1==x_2+t*n_2 --> n_1 *s - n_2 *t == x_2 - x_1 --> http://en.wikipedia.org/wiki/Cramer%27s_rule
      DataType inX = b.X() - c.X(); // x value of the normal vector of the inner segment (b-c)
      DataType inY = b.Y() - c.Y(); // y value of the normal vector of the inner segment (b-c)
      DataType outX = a.X() - b.X(); // x value of the normal vector of the outer segment (a-b)
      DataType outY = a.Y() - b.Y(); // y value of the normal vector of the outer segment (a-b)

      //searching solution for Ax = b, aij are the matrix elements of A, bi are elements of b
      DataType a11 = inY;
      DataType a12 = -inX;
      DataType a21 = -outY;
      DataType a22 = outX;
      DataType b1 = b.X() + outX * 0.5 - (c.X() + inX * 0.5);
      DataType b2 = b.Y() + outY * 0.5 - (c.Y() + inY * 0.5);

      if (a11 * a22 == a12 * a21) { throw Straight_Line(); }

      DataType s = (b1 * a22 - b2 * a21) / (a11 * a22 - a12 * a21); //the determinant is zero iff the three hits are on a line in (x,y).

      return B2Vector3<DataType>(c.X() + inX * 0.5 + s * inY, c.Y() + inY * 0.5 - s * inX, 0.);
    }


    /** calculates calculates the sign of the curvature of given 3-hit-tracklet.
     * a positive value represents a left-oriented curvature, a negative value means having a right-oriented curvature.
     * first vector should be outer hit, second = center hit, third is inner hit */
    template<class PointType, class DataType>
    int calcSign(const PointType& a, const PointType& b, const PointType& c)
    {
      using boost::math::sign;
      B2Vector3<DataType> ba = doAMinusB(a, b); ba.SetZ(0.);
      B2Vector3<DataType> bc = doAMinusB(b, c); bc.SetZ(0.);
      return sign(bc.Orthogonal() * ba); //normal vector of m_vecBC times segment of ba
    }


    /** calculates calculates the sign of the curvature of given 3-hit-tracklet.
     * +1 represents a left-oriented curvature, -1 means having a right-oriented curvature.
     * 0 means it is approximately straight.
     * first vector should be outer hit, second = center hit, third is inner hit */
    template<class PointType, class DataType>
    int calcSign(const PointType& a, const PointType& b, const PointType& c, const B2Vector3<DataType>& sigma_a,
                 const B2Vector3<DataType>& sigma_b, const B2Vector3<DataType>& sigma_c)
    {
      B2Vector3<DataType> c2b = doAMinusB(b, c);   c2b.SetZ(0.);
      B2Vector3<DataType> b2a = doAMinusB(a, b);   b2a.SetZ(0.);
      DataType angle = atan2(b2a[0], b2a[1]) - atan2(c2b[0], c2b[1]);
      //TODO 1/3...mean of the sigmas. Possible improvement: Use a parameter instead, and determine with simulated events.
      DataType sigmaan = (sigma_a.Mag() + sigma_b.Mag() + sigma_c.Mag()) / (3.*(c2b.Mag() + b2a.Mag()));
      if (angle < (-sigmaan)) { return -1; }
      else if (angle > sigmaan)  {return 1; }
      else  { return 0; }
    }
  };
}
