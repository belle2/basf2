/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <math.h>

#include <framework/geometry/B2Vector3.h>
#include <boost/math/special_functions/sign.hpp>
#include <framework/core/FrameworkExceptions.h>


namespace Belle2 {

  /** contains a collection of functions and related stuff needed for SelectionVariables implementing 2-, 3- and 4-hitfilters.
   * Template PointType expects a class having the functions X(), Y() and Z() returning Datatype.
   * Template DataType should behave like a primitive like double or float. */
  template<class PointType, class DataType>
  struct SelVarHelper {

    /** this exception is thrown by the CircleFit and occurs when the track is too straight. */
    /** TODO: remove
    BELLE2_DEFINE_EXCEPTION(Straight_Line, "The hits are on a straight Line (or indistinguishably near to being on it).");
    */

    /** this exception catches TCs which are too small to be able to be detected by the TC, therefore are likely to be ghost TCs. such TCs can be neglected. */
    /** removed: this should be handled by proper setting of the cutoffs:
    BELLE2_DEFINE_EXCEPTION(Circle_too_small,
                            "The radius (%1%) of the circle is too small (threshold: %2%) for usefull pT-estimation.");
    */


    /** is factor containing speed of light (c),
     * the magnetic field (b) and the scaling factor s for conversion of meter in cm : c*b/100 = c*b*s.
     * The standard value assumes a magnetic field of 1.5 Tesla.
     * Value can be changed using the resetMagneticField-Function, where a new value for the magnetic field in Tesla has to be passed.
     * TODO WARNING hardcoded value!  */
    static DataType s_MagneticFieldFactor; //  = 1.5 * 0.00299710;


    /** Overrides standard-Setup for magnetic field.
     *
     * If no value is given, magnetic field is assumed to be Belle2-Detector standard of 1.5T.
     * pT[GeV/c] = 0.299710*B[T]*r[m] = 0.299710*B[T]*r[cm]/100 = 0.00299710B[T]*r[cm].
     */
    static void resetMagneticField(DataType magneticFieldStrength = 1.5) { SelVarHelper::s_MagneticFieldFactor = magneticFieldStrength * 0.00299710; }


    /** checks if results are nan or inf and return 0 if they are, if not, return the result itself. */
    /** TODO: remove
    static DataType checkValid(DataType result)
    { return (std::isnan(result) || std::isinf(result)) ? DataType(0) : result; }
    */

    /** returns the perpendicular magnitude squared of given pointType. */
    static DataType calcPerp2(const PointType& aHit)
    { return (aHit.X() * aHit.X() + aHit.Y() * aHit.Y()); }


    /** returns the perpendicular magnitude of given pointType. */
    static DataType calcPerp(const PointType& aHit) { return sqrt(calcPerp2(aHit)); }


    /** returns the magnitude squared of given pointType. */
    static DataType calcMag2(const PointType& aHit)
    { return (aHit.X() * aHit.X() + aHit.Y() * aHit.Y() + aHit.Z() * aHit.Z()); }


    /** returns the magnitude of given pointType. */
    static DataType calcMag(const PointType& aHit) { return sqrt(calcMag2(aHit)); }


    /** returns scalar product of given two pointTypes. */
    static DataType calcDot(const PointType& aHit, const PointType& bHit)
    { return (aHit.X() * bHit.X() + aHit.Y() * bHit.Y() + aHit.Z() * bHit.Z()); }


    /** returns B2Vector3 containing point a - b. */
    /** TODO: remove
    static B2Vector3<DataType> doAMinusB(const PointType& a, const PointType& b)
    { return B2Vector3<DataType>(a.X() - b.X(), a.Y() - b.Y(), a.Z() - b.Z()); }
    */

    /** calculates the angle of the slope of the hits in RZ, returnValue = theta = atan(r/z).
    * WARNING: returns 0 if no valid value could be found! */
    /** TODO:remove
    static DataType calcSlopeRZ(const PointType& outerHit, const PointType& innerHit)
    {
      double result = atan(
                        sqrt(std::pow(double(outerHit.X() - innerHit.X()), 2)
                             + std::pow(double(outerHit.Y() - innerHit.Y()), 2)
                            ) / double(outerHit.Z() - innerHit.Z())
                      );
      return checkValid(result);
    }
    */


    /** calculates the angle between the hits/vectors (2D), generalized, returning unit: none (incomplete calculation). */
    /** TODO: remove
    static DataType calcAngle2D(const PointType& vecA, const PointType& vecB)
    {
      DataType result = ((vecA.X() * vecB.X() + vecA.Y() * vecB.Y()) / sqrt(calcPerp2(vecA) * calcPerp2(vecB)));
      return (std::isnan(result) || std::isinf(result)) ? DataType(0) : result;
    }
    */

    /** calculates the angle between the hits/vectors (2D), generalized, returning unit: none (incomplete calculation). */
    /* TODO: remove
    static DataType calcAngle2D(const B2Vector3<DataType>& vecA, const B2Vector3<DataType>& vecB)
    {
      DataType result = ((vecA.X() * vecB.X() + vecA.Y() * vecB.Y()) / sqrt(vecA.Perp2() * vecB.Perp2()));
      return (std::isnan(result) || std::isinf(result)) ? DataType(0) : result;
    }
    */

    /** calculates the angle between the hits/vectors (2D), generalized, returning unit: angle in radians.
     */
    /** TODO: remove
    static DataType fullAngle2D(const PointType& vecA, const PointType& vecB)
    { return acos(calcAngle2D(vecA, vecB)); }
    */

    /** calculates the angle between the hits/vectors (2D), generalized, returning unit: angle in radians.
    */
    /** TODO : remove
    static DataType fullAngle2D(const B2Vector3<DataType>& vecA, const B2Vector3<DataType>& vecB)
    { return acos(calcAngle2D(vecA, vecB)); }
    */

    /** calculates an estimation of the radius of given hits and existing estimation of circleCenter (cCenter), returning unit: radius in [cm] (positive value). */
    /** TODO: remove
    static DataType calcRadius(const PointType& a, const PointType& b, const PointType& c, const B2Vector3<DataType>& cCenter)
    {
      return (sqrt(std::pow(cCenter.X() - a.X(), 2) + std::pow(cCenter.Y() - a.Y(), 2)) +
              sqrt(std::pow(cCenter.X() - b.X(), 2) + std::pow(cCenter.Y() - b.Y(), 2)) +
              sqrt(std::pow(cCenter.X() - c.X(), 2) + std::pow(cCenter.Y() - c.Y(), 2))) / 3.;
    } // = radius in [cm], sign here not needed. normally: signKappaAB/normAB1
    */

    /** calculates an estimation of circleCenter position, result is returned as the x and y value of the B2Vector3. */
    /** TODO: remove
    static B2Vector3<DataType> calcCircleCenter(const PointType& a, const PointType& b, const PointType& c) throw(Straight_Line)
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

      DataType s = (b1 * a22 - b2 * a21) / (a11 * a22 - a12 * a21); //the determinant is zero if the three hits are on a line in (x,y).

      return B2Vector3<DataType>(c.X() + inX * 0.5 + s * inY, c.Y() + inY * 0.5 - s * inX, 0.);
    }
    */

    /** calculates the estimation of the transverse momentum of given radius using defined strength of magnetic field, returning unit: GeV/c. */
    static DataType calcPt(DataType radius, DataType field = SelVarHelper::s_MagneticFieldFactor)
    {
      // TODO WARNING hardcoded value, is there a quasi-global value for such cases (this case, minimal accepted radius)
      // if (fabs(radius) < 0.0000001) { throw (Circle_too_small() << radius << 0.0000001); }

      //WARNING: does not care for negative radius!!
      return field * radius;
    } // return unit: GeV/c


    /** calculates calculates the sign of the curvature of given 3-hit-tracklet.
     * a positive value represents a left-oriented curvature, a negative value means having a right-oriented curvature.
    * 0 means that it is exactly straight or that two hits are identical.
     * first vector should be outer hit, second = center hit, third is inner hit. */
    /** TODO: remove
    static int calcSign(const PointType& a, const PointType& b, const PointType& c)
    {
      using boost::math::sign;
      B2Vector3<DataType> ba(a.X() - b.X(), a.Y() - b.Y(), 0.0);
      B2Vector3<DataType> bc(b.X() - c.X(), b.Y() - c.Y(), 0.0);
      return sign(bc.Orthogonal() * ba); //normal vector of m_vecBC times segment of ba
    }
    */

    /** calculates calculates the sign of the curvature of given 3-hit-tracklet.
     * +1 represents a left-oriented curvature, -1 means having a right-oriented curvature.
     * 0 means it is approximately straight.
     * first vector should be outer hit, second = center hit, third is inner hit. */
    /** TODO: remove
    static int calcSign(const PointType& a, const PointType& b, const PointType& c, const B2Vector3<DataType>& sigma_a,
                        const B2Vector3<DataType>& sigma_b, const B2Vector3<DataType>& sigma_c)
    {
      B2Vector3<DataType> c2b(b.X() - c.X(), b.Y() - c.Y(), 0.0);
      B2Vector3<DataType> b2a(a.X() - b.X(), a.Y() - b.Y(), 0.0);
      DataType angle = atan2(b2a[0], b2a[1]) - atan2(c2b[0], c2b[1]);
      //TODO 1/3...mean of the sigmas. Possible improvement: Use a parameter instead, and determine with simulated events.
      DataType sigmaan = (sigma_a.Mag() + sigma_b.Mag() + sigma_c.Mag()) / (3.*(c2b.Mag() + b2a.Mag()));
      if (angle < (-sigmaan)) { return -1; }
      else if (angle > sigmaan)  {return 1; }
      else  { return 0; }
    }
    */

  };

  template<typename PointType, typename DataType> DataType SelVarHelper<PointType, DataType>::s_MagneticFieldFactor = 1.5 *
      0.00299710;

}
