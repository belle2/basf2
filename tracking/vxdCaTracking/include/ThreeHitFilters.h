/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TVector3.h>
#include "tracking/vxdCaTracking/TwoHitFilters.h"
#include "tracking/vxdCaTracking/FilterExceptions.h"
#include <framework/logging/Logger.h>

namespace Belle2 {

  /** The class 'ThreeHitFilters' bundles filter methods using 3 hits which are stored in TVector3s. */
  class ThreeHitFilters {
  public:

    /** Empty constructor. For initialisation only, an object generated this way is useless unless resetValues(a, b) is called at least once */
    ThreeHitFilters():
      m_circleCenterCalculated(false),
      m_radiusCalculated(false),
      m_radius(0.),
      m_x2(0.),
      m_y2(0.),
      m_z2(0.)
    {
      m_hitA.SetXYZ(0., 0., 0.);
      m_hitB.SetXYZ(0., 0., 0.);
      m_hitC.SetXYZ(0., 0., 0.);
      m_vecAB.SetXYZ(0., 0., 0.);
      m_vecBC.SetXYZ(0., 0., 0.);
      resetMagneticField(1.5);
    }

    /** Constructor. needs the first parameter is outer hit, second is center hit, third is inner hit. Parameters in TVector3-format, Optional parameter is the strength of the magnetic field in Tesla*/
    ThreeHitFilters(const TVector3& outerHit, const TVector3& centerHit, const TVector3& innerHit,
                    const double magneticFieldStrength = 1.5):
      m_circleCenterCalculated(false),
      m_radiusCalculated(false),
      m_radius(0.),
      m_hitA(outerHit),
      m_hitB(centerHit),
      m_hitC(innerHit),
      m_vecAB(outerHit - centerHit),
      m_vecBC(centerHit - innerHit)
    {
      m_x2 = m_vecAB[0] * m_vecBC[0]; /// x-part of m_vecAB.Dot(m_vecBC)
      m_y2 = m_vecAB[1] * m_vecBC[1]; /// y-part of m_vecAB.Dot(m_vecBC)
      m_z2 = m_vecAB[2] * m_vecBC[2]; /// z-part of m_vecAB.Dot(m_vecBC)
      resetMagneticField(magneticFieldStrength);
    }


    /** Destructor. */
    ~ThreeHitFilters() {}

    /** Overrides Constructor-Setup. Needed if you want to reuse the instance instead of recreating one */
    void resetValues(const TVector3& outerHit, const TVector3& centerHit, const TVector3& innerHit)
    {
      m_radiusCalculated = false;
      m_circleCenterCalculated = false;
      m_radius = 0.;
      m_hitA = outerHit;
      m_hitB = centerHit;
      m_hitC = innerHit;
      m_vecAB = outerHit - centerHit;
      m_vecBC = centerHit - innerHit;

      m_x2 = m_vecAB[0] * m_vecBC[0]; /// x-part of m_vecAB.Dot(m_vecBC)
      m_y2 = m_vecAB[1] * m_vecBC[1]; /// y-part of m_vecAB.Dot(m_vecBC)
      m_z2 = m_vecAB[2] * m_vecBC[2]; /// z-part of m_vecAB.Dot(m_vecBC)
    }



    /** Overrides Constructor-Setup for magnetic field.
     *
     * If no value is given, magnetic field is assumed to be Belle2-Detector standard of 1.5T.
     * pT[GeV/c] = 0.299710*B[T]*r[m] = 0.299710*B[T]*r[cm]/100 = 0.00299710B[T]*r[cm]
     */
    void resetMagneticField(const double magneticFieldStrength = 1.5) { m_magneticFieldFactor = magneticFieldStrength * 0.00299710; }



    /** returns the set value of the magnetic field in Tesla */
    double getMagneticField() { return m_magneticFieldFactor / 0.00299710; }



    /** returns zero if value is nan or inf */
    double filterNan(double value) { return m_twoHitFilter.filterNan(value); }



    /** calculates the angle between the hits/vectors (3D), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use fullAngle3D instead) */
    double calcAngle3D()
    {
      double angle = ((m_x2 + m_y2 + m_z2) / (m_vecAB.Mag2() *
                                              m_vecBC.Mag2())); // fullCalc would be acos(m_vecAB.Dot(m_vecBC) / m_vecAB.Mag()*m_vecBC.Mag())
      return filterNan(angle);
    } // return unit: none (calculation for degrees is incomplete, if you want readable numbers, use fullAngle3D instead)



    /** calculates the angle between the hits/vectors (3D), returning unit: angle in degrees */
    double fullAngle3D()
    {
      double angle = acos(m_vecAB.Dot(m_vecBC) / (m_vecAB.Mag() * m_vecBC.Mag())); // 0-pi
      angle = (angle * (180. / M_PI));
      return filterNan(angle);
    } // return unit: ° (0 - 180°)



    /** calculates the angle between the hits/vectors (XY), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use fullAngleXY instead) */
    double calcAngleXY()
    {
      double angle = ((m_x2 + m_y2) / (m_vecAB.Perp2() * m_vecBC.Perp2())); // fullAngle:
      return filterNan(angle);
    } // return unit: none (calculation for degrees is incomplete, if you want readable numbers, use fullAngleXY instead)



    /** calculates the angle between the hits/vectors (XY), returning unit: angle in degrees */
    double fullAngleXY()
    {
      double angle = fullAngle2D(m_vecAB, m_vecBC); // 0-pi
      angle = (angle * (180. / M_PI));
      return filterNan(angle);
    } // return unit: ° (0 - 180°)



    /** calculates the angle between the hits/vectors (RZ), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use fullAngleRZ instead) */
    double calcAngleRZ()
    {
      TVector3 rzVecAB(m_vecAB.Perp(), m_vecAB[2], 0.);
      TVector3 rzVecBC(m_vecBC.Perp(), m_vecBC[2], 0.);
      return calcAngle2D(rzVecAB, rzVecBC);
    } // return unit: none (calculation for degrees is incomplete, if you want readable numbers, use fullAngleRZ instead)



    /** calculates the angle between the hits/vectors (RZ), returning unit: angle in degrees */
    double fullAngleRZ()
    {
      TVector3 rzVecAB(m_vecAB.Perp(), m_vecAB[2], 0.);
      TVector3 rzVecBC(m_vecBC.Perp(), m_vecBC[2], 0.);
      double angle = fullAngle2D(rzVecAB, rzVecBC); // 0-pi
      angle = (angle * (180. / M_PI));
      return filterNan(angle);
    } // return unit: ° (0 - 180°)



    /** calculates the distance of the point of closest approach of circle to the IP, returning unit: cm */
    double calcCircleDist2IP()
    {
      checkCalcRadius();

      return (fabs(m_centerABC.Perp() - m_radius)); // distance of closest approach of circle to the IP
    } // return unit: cm



    /** calculates the estimation of the transverse momentum of the 3-hit-tracklet, returning unit: GeV/c */
    double calcPt()
    {
      checkCalcRadius();

      return calcPt(m_radius);
    } // return unit: GeV/c



    /** calculates the estimation of the transverse momentum of given radius using defined strength of magnetic field, returning unit: GeV/c */
    double calcPt(double radius)
    {
      sanityCheckRadius(radius);

      return (m_magneticFieldFactor * radius);
    } // return unit: GeV/c



    /** calculates deviations in the slope of the inner segment and the outer segment, returning unit: none */
    double calcDeltaSlopeRZ()
    {
      m_twoHitFilter.resetValues(m_hitA, m_hitB);
      double slopeAB = m_twoHitFilter.calcSlopeRZ();
      m_twoHitFilter.resetValues(m_hitB, m_hitC);
      double slopeBC = m_twoHitFilter.calcSlopeRZ();

      return filterNan(slopeBC - slopeAB);
    } // return unit: none



    /** calculates the helixparameter describing the deviation in arc length per unit in z.
     *
     * returning unit: radians*cm
     */
    double calcDeltaSOverZ()
    {
      checkCalcCircleCenter();

      TVector3 points2hitA = m_hitA - m_centerABC;
      TVector3 points2hitB = m_hitB - m_centerABC;
      TVector3 points2hitC = m_hitC - m_centerABC;
      double alfaAB = fullAngle2D(points2hitA, points2hitB);
      double alfaBC = fullAngle2D(points2hitB, points2hitC);
      //return filterNan( (alfaAB * m_vecBC[2]) - (alfaBC *m_vecAB[2]) );
      return (alfaAB * m_vecBC[2]) - (alfaBC *
                                      m_vecAB[2]); // equals to alfaAB/dZAB and alfaBC/dZBC, but this solution here can not produce a division by zero
    } // return unit: radians*cm



    /** compares the "slopes" z over arc length. calcDeltaSlopeZOverS is invariant under rotations in the r-z plane. */
    double calcDeltaSlopeZOverS()
    {
      checkCalcRadius();

      TVector3 points2hitA = m_hitA - m_centerABC;
      TVector3 points2hitB = m_hitB - m_centerABC;
      TVector3 points2hitC = m_hitC - m_centerABC;
      double alfaABr = fullAngle2D(points2hitA, points2hitB) * m_radius;
      double alfaBCr = fullAngle2D(points2hitB, points2hitC) * m_radius;

      return filterNan((asin(m_vecAB[2] / alfaABr)) - asin(m_vecBC[2] / alfaBCr)); // Beware of z>r!
    }



    /** calculates the helixparameter describing the deviation in z per unit angle, returning unit: none */
    double calcHelixParameterFit()
    {
      checkCalcCircleCenter();
      TVector3 points2hitA = m_hitA - m_centerABC;
      TVector3 points2hitB = m_hitB - m_centerABC;
      TVector3 points2hitC = m_hitC - m_centerABC;
      double alfaAB = calcAngle2D(points2hitA, points2hitB);
      double alfaBC = calcAngle2D(points2hitB, points2hitC);
      // real calculation: ratio is (m_vecij[2] = deltaZ): alfaAB/deltaZab : alfaBC/deltaZbc, the following equation saves two times '/'
      return filterNan((alfaAB * m_vecBC[2]) / (alfaBC * m_vecAB[2]));
    } // return unit: none



    /** reverse compatibility, calls calcHelixParameterFit */
    double calcHelixFit() { return calcHelixParameterFit(); }



    /** calculates the angle between the hits/vectors (2D), generalized, returning unit: none. used by calcAngleRZ and calcHelixFit (angleXY could use it too, but this one profits from other optimizations instead) */
    double calcAngle2D(const TVector3& vecA, const TVector3& vecB)
    {
      double angle = ((vecA[0] * vecB[0] + vecA[1] * vecB[1]) / sqrt(vecA.Perp2() * vecB.Perp2()));
      return filterNan(angle);
    }

    /** calculates the angle between the hits/vectors (2D), generalized, returning unit: angle in radians
     * WARNING it is radians, which is incompatible to fullAngle3D (°))
     */
    double fullAngle2D(const TVector3& vecA, const TVector3& vecB)
    {
      return acos(calcAngle2D(vecA, vecB));
      //return filterNan(angle);
    }



    /** calculates an estimation of the radius of given hits and existing estimation of circleCenter, returning unit: radius in [cm] (positive value)*/
    double calcRadius(const TVector3& a, const TVector3& b, const TVector3& c, const TVector3& circleCenter)
    {
      return ((circleCenter - a).Perp() + (circleCenter - b).Perp() + (circleCenter - c).Perp()) /
             3.;   // = radius in [cm], sign here not needed. normally: signKappaAB/normAB1
    } // used by calcPt() and calcCircleDist2IP()



    /** calculates an estimation of circleCenter position, result is written into the 4th input-parameter */
    void calcCircleCenter(const TVector3& a, const TVector3& b, const TVector3& c, TVector3& circleCenter)
    {
      // calculates the intersection point using Cramer's rule.
      // x_1+s*n_1==x_2+t*n_2 --> n_1 *s - n_2 *t == x_2 - x_1 --> http://en.wikipedia.org/wiki/Cramer%27s_rule
      double inX = b[0] - c[0]; // x value of the normal vector of the inner segment (b-c)
      double inY = b[1] - c[1]; // y value of the normal vector of the inner segment (b-c)
      double outX = a[0] - b[0]; // x value of the normal vector of the outer segment (a-b)
      double outY = a[1] - b[1]; // y value of the normal vector of the outer segment (a-b)

      //searching solution for Ax = b, aij are the matrix elements of A, bi are elements of b
      double a11 = inY;
      double a12 = -inX;
      double a21 = -outY;
      double a22 = outX;
      double b1 = b[0] + outX * 0.5 - (c[0] + inX * 0.5);
      double b2 = b[1] + outY * 0.5 - (c[1] + inY * 0.5);

      if (a11 * a22 == a12 * a21) { throw FilterExceptions::Straight_Line(); }

      double s = (b1 * a22 - b2 * a21) / (a11 * a22 - a12 * a21); //the determinant is zero iff the three hits are on a line in (x,y).

      circleCenter.SetXYZ(c[0] + inX * 0.5 + s * inY, c[1] + inY * 0.5 - s * inX, 0.);
    }



    /** calculates calculates the sign of the curvature of given 3-hit-tracklet. a positive value represents a left-oriented curvature, a negative value means having a right-oriented curvature. first vector should be outer hit, second = center hit, third is inner hit*/
    int calcSign(const TVector3& a, const TVector3& b, const TVector3& c);



    /** calculates calculates the sign of the curvature of given 3-hit-tracklet. +1 represents a left-oriented curvature, -1 means having a right-oriented curvature. 0 means it is approximately straight. first vector should be outer hit, second = center hit, third is inner hit*/
    int calcSign(const TVector3& a, const TVector3& b, const TVector3& c, const TVector3& sigma_a, const TVector3& sigma_b,
                 const TVector3& sigma_c)
    {
      TVector3 c2b = b - c;   c2b.SetZ(0.);
      TVector3 b2a = a - b;   b2a.SetZ(0.);
      double angle = atan2(b2a[0], b2a[1]) - atan2(c2b[0], c2b[1]);
      double sigmaan = (sigma_a.Mag() + sigma_b.Mag() + sigma_c.Mag()) / (3.*(c2b.Mag() +
                       b2a.Mag())); //TODO 1/3...mean of the sigmas. Possible improvement: Use a parameter instead, and determine with simulated events.
      if (angle < (-sigmaan)) { return -1; }
      else if (angle > sigmaan)  {return 1; }
      else  { return 0; }
    }



  protected:


    /** checks whether the calcCircleCenter()-Member has been executed already and executes it if not */
    void checkCalcCircleCenter()
    {
      if (m_circleCenterCalculated == false) {
        calcCircleCenter(m_hitA, m_hitB, m_hitC, m_centerABC);
        m_circleCenterCalculated = true;
      }
    }


    /** checks whether the calcRadius()-Member has been executed already and executes it if not */
    void checkCalcRadius()
    {
      checkCalcCircleCenter();
      if (m_radiusCalculated == false) {
        m_radius = calcRadius(m_hitA, m_hitB, m_hitC, m_centerABC);
        sanityCheckRadius(m_radius);
        m_radiusCalculated = true;
      }
    }


    /** check Radius for bad values and throw exception if the value is bad */
    void sanityCheckRadius(double radius)
    {
      if (fabs(radius) <
          0.0000001) { // WARNING hardcoded value, is there a quasi-global value for such cases (this case, minimal accepted radius)
        m_radiusCalculated = false;
        m_circleCenterCalculated = false;
        m_radius = 0.;
        B2ERROR("sanityCheckRadius: given radius is " << radius << ", which is below threshold of " << 0.0000001 << ", throwing exception");
        throw FilterExceptions::Circle_too_small();
      }
    }



    TwoHitFilters m_twoHitFilter; /**< instance of TwoHitFilters-class used for some internal calculations */

    bool m_circleCenterCalculated; /**< initially set to false, will be set true if calcCircleCenter() is used at least once */
    bool m_radiusCalculated; /**< initially set to false, will be set true if calcInvCurvature() is used at least once */
    double m_radius; /**< radius[cm] of a circle in r-phi-plane formed by the 3 hits */
    double m_x2; /**< internal intermediate value storing x^2, no enduser-relevance */
    double m_y2; /**< internal intermediate value storing y^2, no enduser-relevance */
    double m_z2; /**< internal intermediate value storing z^2, no enduser-relevance */
    double m_magneticFieldFactor; /**< is factor containing speed of light (c), the magnetic field (b) and the scaling factor s for conversion of meter in cm : c*b/100 = c*b*s */
    TVector3 m_centerABC;  /**< center position of a circle in r-phi-plane formed by the 3 hits */
    TVector3 m_hitA; /**< outer hit (position relevant for useful filter calculation) used for the filter calculation */
    TVector3 m_hitB; /**< center hit (position relevant for useful filter calculation) used for the filter calculation */
    TVector3 m_hitC; /**< inner hit (position relevant for useful filter calculation) used for the filter calculation */
    TVector3 m_vecAB; /**< vector pointing from center hit to outer hit (outer segment) */
    TVector3 m_vecBC; /**< vector pointing from inner hit to center hit (inner segment) */

  }; //end class ThreeHitFilters
} //end namespace Belle2
