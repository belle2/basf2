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

#include <tracking/vxdHoughTracking/filters/pathFilters/TwoHitVariables.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/geometry/B2Vector3.h>

#include <cmath>

namespace Belle2 {
  namespace vxdHoughTracking {

    /// Class that allows the calculation of simple variables to estimate the quality of a triplet of hits.
    class ThreeHitVariables {
    public:
      /// basic constructor
      ThreeHitVariables() : m_oHit(0., 0., 0.), m_cHit(0., 0., 0.), m_iHit(0., 0., 0.),
        m_outerDifferenceVector(0., 0., 0.), m_innerDifferenceVector(0., 0., 0.)
      {};

      /// actual useful constructor
      /// @param oHit B2Vector3D of the outer hit used for calculating the single variables
      /// @param cHit B2Vector3D of the center hit used for calculating the single variables
      /// @param iHit B2Vector3D of the inner hit used for calculating the single variables
      ThreeHitVariables(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit) :
        m_oHit(oHit), m_cHit(cHit), m_iHit(iHit)
      {
        m_outerDifferenceVector = oHit - cHit;
        m_innerDifferenceVector = cHit - iHit;
      };

      /// Set hits if not given in constructor of if they need to be changed.
      /// @param oHit B2Vector3D of the outer hit used for calculating the single variables
      /// @param cHit B2Vector3D of the center hit used for calculating the single variables
      /// @param iHit B2Vector3D of the inner hit used for calculating the single variables
      void setHits(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit)
      {
        m_oHit = oHit;
        m_oHit = oHit;
        m_iHit = iHit;
        m_outerDifferenceVector = oHit - cHit;
        m_innerDifferenceVector = cHit - iHit;
      }

      /// helper function which calculates the average distance in XY from the given center
      /// @param circleCenter center of the cirlce for which the average distance is calculated
      /// returns the average distance in cm of the hits to the circle center position
      double calcAvgDistanceXY(const B2Vector3D& circleCenter)
      {
        return (sqrt(std::pow(circleCenter.X() - m_oHit.X(), 2) + std::pow(circleCenter.Y() - m_oHit.Y(), 2)) +
                sqrt(std::pow(circleCenter.X() - m_cHit.X(), 2) + std::pow(circleCenter.Y() - m_cHit.Y(), 2)) +
                sqrt(std::pow(circleCenter.X() - m_iHit.X(), 2) + std::pow(circleCenter.Y() - m_iHit.Y(), 2))) / 3.;
      }


      /// calculates the angle between the hits/vectors (3D), returning unit: angle in radian
      double getAngle3D()
      {
        double result = acos(m_outerDifferenceVector.Dot(m_innerDifferenceVector) /
                             (m_outerDifferenceVector.Mag() * m_innerDifferenceVector.Mag())); // 0-pi
        return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
      }


      /// calculates the angle between the hits/vectors (3D), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use Angle3DFull instead)
      double getAngle3DSimple()
      {
        // fullCalc would be acos(m_vecAB.Dot(m_vecBC) / m_vecAB.Mag()*m_vecBC.Mag()), but here time-consuming parts have been neglected
        double result = m_outerDifferenceVector.Dot(m_innerDifferenceVector) /
                        (m_outerDifferenceVector.Mag2() * m_innerDifferenceVector.Mag2());
        return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
      }


      /// calculates the angle between the hits/vectors (RZ), returning unit: angle in radian
      double getAngleRZ()
      {
        B2Vector3D rzVecAB(m_outerDifferenceVector.Perp(), m_outerDifferenceVector.Z(), 0.);
        B2Vector3D rzVecBC(m_innerDifferenceVector.Perp(), m_innerDifferenceVector.Z(), 0.);
        TwoHitVariables twoHitVariables(rzVecAB, rzVecBC);

        double result = acos(twoHitVariables.getCosXY());  // 0-pi
        return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
      } // return unit: rad (0 - pi)


      /// calculates the cosine of the angle between the hits/vectors (RZ), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleRZFull instead)
      double getCosAngleRZSimple()
      {
        B2Vector3D rzVecAB(m_outerDifferenceVector.Perp(), m_outerDifferenceVector.Z(), 0.);
        B2Vector3D rzVecBC(m_innerDifferenceVector.Perp(), m_innerDifferenceVector.Z(), 0.);
        TwoHitVariables twoHitVariables(rzVecAB, rzVecBC);

        return twoHitVariables.getCosXY();
      }

      /// Calculates the angle in x-y between two vectors
      /// return unit: rad (0 - pi)
      double getAngleXY()
      {
        TwoHitVariables twoHitVariables(m_outerDifferenceVector, m_innerDifferenceVector);
        double result = acos(twoHitVariables.getCosXY());   // 0-pi
        return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
      }


      /// calculates an estimation of circleCenter position, result is returned as the x and y value of the B2Vector3.
      B2Vector3D getCircleCenterXY()
      {
        // calculates the intersection point using Cramer's rule.
        // x_1+s*n_1==x_2+t*n_2 --> n_1 *s - n_2 *t == x_2 - x_1 --> http://en.wikipedia.org/wiki/Cramer%27s_rule
        double inX = m_cHit.X() - m_iHit.X(); // x value of the normal vector of the inner segment (m_cHit-m_iHit)
        double inY = m_cHit.Y() - m_iHit.Y(); // y value of the normal vector of the inner segment (m_cHit-m_iHit)
        double outX = m_oHit.X() - m_cHit.X(); // x value of the normal vector of the outer segment (m_oHit-m_cHit)
        double outY = m_oHit.Y() - m_cHit.Y(); // y value of the normal vector of the outer segment (m_oHit-m_cHit)

        // searching solution for Ax = b, aij are the matrix elements of A, bi are elements of b
        double a11 = inY;
        double a12 = -inX;
        double a21 = -outY;
        double a22 = outX;
        double b1 = m_cHit.X() + outX * 0.5 - (m_iHit.X() + inX * 0.5);
        double b2 = m_cHit.Y() + outY * 0.5 - (m_iHit.Y() + inY * 0.5);

        // protect against the determinant being zero
        if (a11 * a22 == a12 * a21) {
          return B2Vector3D(1e30, 1e30, 1e30);
        }

        // the determinant is zero if the three hits are on a line in (x,y), which is checked above.
        double s = (b1 * a22 - b2 * a21) / (a11 * a22 - a12 * a21);

        return B2Vector3D(m_iHit.X() + inX * 0.5 + s * inY, m_iHit.Y() + inY * 0.5 - s * inX, 0.);
      }


      /// calculates the distance of the point of closest approach of circle to the IP, returning unit: cm
      double getCircleDistanceIP()
      {
        B2Vector3D circleCenter = getCircleCenterXY();
        if (circleCenter.Perp2() > 1e30) {
          return NAN;
        }
        double circleRadius = calcAvgDistanceXY(circleCenter);

        // distance of closest approach of circle to the IP :
        // WARNING only valid for IP=0,0,X
        return (fabs(circleCenter.Perp() - circleRadius));
      }


      /// calculates the estimation of the circle radius of the 3-hit-tracklet, returning unit: cm.
      double getCircleRadius()
      {
        B2Vector3D circleCenter = getCircleCenterXY();
        if (circleCenter.Perp2() > 1e30) {
          return NAN;
        }
        return calcAvgDistanceXY(circleCenter);
      }


      /// calculates the angle between the hits/vectors (XY),
      /// returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleXYFull instead)
      double getCosAngleXY()
      {
        double result = (m_outerDifferenceVector.X() * m_innerDifferenceVector.X() +
                         m_outerDifferenceVector.Y() * m_innerDifferenceVector.Y()) /
                        (m_outerDifferenceVector.Perp() * m_innerDifferenceVector.Perp());

        return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
      }


      /// calculates deviations in the slope of the inner segment and the outer segment, returning unit: none
      double getDeltaSlopeRZ()
      {
        TwoHitVariables outerTwoHitVariables(m_oHit, m_cHit);
        TwoHitVariables innerTwoHitVariables(m_cHit, m_iHit);
        double slopeOC = outerTwoHitVariables.getRZSlope();
        double slopeCI = innerTwoHitVariables.getRZSlope();

        return slopeCI - slopeOC;
      }


      /// compares the "slopes" z over arc length. calcDeltaSlopeZOverS is invariant under rotations in the r-z plane.
      double getDeltaSlopeZoverS()
      {
        B2Vector3D circleCenter = getCircleCenterXY();
        if (circleCenter.Perp2() > 1e30) {
          return NAN;
        }
        double circleRadius = calcAvgDistanceXY(circleCenter);
        B2Vector3D vecOuter2cC  = m_oHit - circleCenter;
        B2Vector3D vecCenter2cC = m_cHit - circleCenter;
        B2Vector3D vecInner2cC  = m_iHit - circleCenter;

        TwoHitVariables outerTwoHitVariables(vecOuter2cC, vecCenter2cC);
        TwoHitVariables innerTwoHitVariables(vecCenter2cC, vecInner2cC);

        // WARNING: this is only approximately S (valid in the limit of small angles) but might be OK for this use!!!
        //  want to replace id with 2*sin ( alfa ) * circleRadius
        double alfaOCr = acos(outerTwoHitVariables.getCosXY()) * circleRadius ;
        double alfaCIr = acos(innerTwoHitVariables.getCosXY()) * circleRadius ;

        // Beware of z>r!:
        double result = (asin(double(m_oHit.Z() - m_cHit.Z()) / alfaOCr)) - asin(double(m_cHit.Z() - m_iHit.Z()) / alfaCIr);

        return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
      }


      /// calculates the helixparameter describing the deviation in arc length per unit in z. returning unit: radians*cm
      double getDeltaSoverZ()
      {
        B2Vector3D circleCenter = getCircleCenterXY();
        if (circleCenter.Perp2() > 1e30) {
          return NAN;
        }
        B2Vector3D vecOuter2cC  = m_oHit - circleCenter;
        B2Vector3D vecCenter2cC = m_cHit - circleCenter;
        B2Vector3D vecInner2cC  = m_iHit - circleCenter;

        TwoHitVariables outerTwoHitVariables(vecOuter2cC, vecCenter2cC);
        TwoHitVariables innerTwoHitVariables(vecCenter2cC, vecInner2cC);
        double alfaOC = acos(outerTwoHitVariables.getCosXY());
        double alfaCI = acos(innerTwoHitVariables.getCosXY());

        // equals to alfaAB/dZAB and alfaBC/dZBC, but this solution here can not produce a division by zero:
        return (alfaOC * double(m_cHit.Z() - m_iHit.Z())) - (alfaCI * double(m_oHit.Z() - m_cHit.Z()));
      }


      /// calculates the helixparameter describing the deviation in z per unit angle, returning unit: none
      double performHelixParamterFit()
      {
        B2Vector3D circleCenter = getCircleCenterXY();
        if (circleCenter.Perp2() > 1e30) {
          return NAN;
        }

        B2Vector3D vecOuter2cC  = m_oHit - circleCenter;
        B2Vector3D vecCenter2cC = m_cHit - circleCenter;
        B2Vector3D vecInner2cC  = m_iHit - circleCenter;
        TwoHitVariables outerTwoHitVariables(vecOuter2cC, vecCenter2cC);
        TwoHitVariables innerTwoHitVariables(vecCenter2cC, vecInner2cC);

        double alfaAB = outerTwoHitVariables.getCosXY();
        double alfaBC = innerTwoHitVariables.getCosXY();

        // real calculation: ratio is (m_vecij[2] = deltaZ): alfaAB/deltaZab : alfaBC/deltaZbc, the following equation saves two times '/'
        double result = (alfaAB * double(m_cHit.Z() - m_iHit.Z())) / (alfaBC * double(m_oHit.Z() - m_cHit.Z()));

        return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
      }


      /// calculates the estimation of the transverse momentum of the 3-hit-tracklet, returning unit: GeV/c
      double getSimplePTEstimate()
      {
        B2Vector3D circleCenter = getCircleCenterXY();
        if (circleCenter.Perp2() > 1e30) {
          return NAN;
        }
        double circleRadius = calcAvgDistanceXY(circleCenter);

        return 0.00299792458 * m_BFieldZ * circleRadius;
      }


      /// calculates calculates the sign of the curvature of given 3-hit-tracklet.
      /// A positive value represents a left-oriented curvature, a negative value means having a right-oriented curvature.
      /// 0 means that it is exactly straight or that two hits are identical.
      int getCurvatureSign()
      {
        using boost::math::sign;
        B2Vector3D ba(m_oHit.X() - m_cHit.X(), m_oHit.Y() - m_cHit.Y(), 0.0);
        B2Vector3D bc(m_cHit.X() - m_iHit.X(), m_cHit.Y() - m_iHit.Y(), 0.0);
        return sign(bc.Orthogonal() * ba); //normal vector of m_vecBC times segment of ba
      }

      /// calculates calculates the sign of the curvature of 3-hit-tracklet given as arguments.
      /// A positive value represents a left-oriented curvature, a negative value means having a right-oriented curvature.
      /// 0 means that it is exactly straight or that two hits are identical.
      int getCurvatureSign(const B2Vector3D& oHit, const B2Vector3D& cHit, const B2Vector3D& iHit)
      {
        using boost::math::sign;
        B2Vector3D ba(oHit.X() - cHit.X(), oHit.Y() - cHit.Y(), 0.0);
        B2Vector3D bc(cHit.X() - iHit.X(), cHit.Y() - iHit.Y(), 0.0);
        return sign(bc.Orthogonal() * ba); //normal vector of m_vecBC times segment of ba
      }

      /// Set the B-Field value used for pT calculations
      /// @param bfieldZ B-Field value to be used
      void setBFieldZ(const double bfieldZ = 1.5) { m_BFieldZ = bfieldZ; }

    private:
      /// BField along z to estimate pT
      double m_BFieldZ = 1.5;
      /// outermost hit position
      B2Vector3D m_oHit;
      /// center hit position
      B2Vector3D m_cHit;
      /// innermost hit position
      B2Vector3D m_iHit;
      /// The following two differences are used very often, so calculate once on construction
      /// vector containing the difference m_oHit - m_cHit
      B2Vector3D m_outerDifferenceVector;
      /// vector containing the difference m_cHit - m_iHit
      B2Vector3D m_innerDifferenceVector;

    };
  }
}
