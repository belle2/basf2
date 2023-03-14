/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/geometry/B2Vector3.h>
#include <math.h>



namespace Belle2 {
  /** The class 'TwoHitFilters' bundles filter methods using 2 hits which are stored in B2Vector3Ds. */
  class TwoHitFilters {
  public:

    /** Empty constructor. For pre-inizialisation only, an object generated this way is useless unless resetValues(a, b) is called at least once */
    TwoHitFilters():
      m_x2(0.),
      m_y2(0.),
      m_z2(0.),
      m_dz(0.)
    {
      m_hitA.SetXYZ(0., 0., 0.);
      m_hitB.SetXYZ(0., 0., 0.);
    }

    /** Constructor. needs the first parameter is outer hit, second is inner hit. Parameters in B2Vector3D-format*/
    TwoHitFilters(const B2Vector3D& outerHit, const B2Vector3D& innerHit):
      m_hitA(outerHit),
      m_hitB(innerHit) { initializeMe(outerHit, innerHit); }


    /** Destructor. */
    ~TwoHitFilters() {}

    /** Overrides Constructor-Setup. Needed if you want to reuse the instance instead of recreating one */
    void resetValues(const B2Vector3D& outerHit, const B2Vector3D& innerHit)
    {
      m_hitA = outerHit;
      m_hitB = innerHit;

      initializeMe(outerHit, innerHit);
    }

    /** calculates the distance between the hits (3D), returning unit: cm^2 for speed optimization */
    double calcDist3D() const { return (m_x2 + m_y2 + m_z2); } // return unit: cm^2

    /** calculates the real distance between the hits (3D), returning unit: cm */
    double fullDist3D() const { return sqrt(calcDist3D()); } // return unit: cm

    /** calculates the distance between the hits (XY), returning unit: cm^2 for speed optimization */
    double calcDistXY() const { return (m_x2 + m_y2); } // return unit: cm^2

    /** calculates the real distance between the hits (XY), returning unit: cm */
    double fullDistXY() const { return sqrt(calcDistXY()); } // return unit: cm

    /** calculates the distance between the hits (Z only), returning unit: cm */
    double calcDistZ() const { return m_dz; } // return unit: cm

    /** calculates the angle of the slope of the hits in RZ, returnValue = theta = atan(r/z) */
    double calcSlopeRZ() const
    {
      //double slope = atan (calcDistXY() / m_dz) ;
      double slope = atan(fullDistXY() / m_dz) ;    //since calcDistXY() returns cm ^2
      return filterNan(slope);
    } // return unit: radians

    /** calculates the angle of the slope of the hits in RZ, returnValue = theta = atan(r/z) */
    double fullSlopeRZ() const { return calcSlopeRZ(); }

    /** calculates the normed distance between the hits (3D), return unit: none */
    double calcNormedDist3D() const
    {
      double normedVal = calcDistXY() / calcDist3D();
      return filterNan(normedVal);
    } // return unit: none

    /** nice little nanChecker returns 0 if value was nan or inf, else returns value itself */
    double filterNan(double value) const;

  protected:

    /** initializer function, sets values */
    void initializeMe(const B2Vector3D& outerHit, const B2Vector3D& innerHit)
    {
      m_x2 = outerHit[0] - innerHit[0]; // not x2 yet, reusing member
      m_y2 = outerHit[1] - innerHit[1];
      m_dz = outerHit[2] - innerHit[2];

      m_x2 *= m_x2; // now it's x2...
      m_y2 *= m_y2;
      m_z2 = m_dz * m_dz;
    }

    B2Vector3D
    m_hitA; /**< outer hit (position relevant for useful filter calculation, e.g. calcDist3D) used for the filter calculation */
    B2Vector3D
    m_hitB; /**< inner hit (position relevant for useful filter calculation, e.g. calcDist3D) used for the filter calculation */
    double m_x2; /**< internal intermediate value storing x^2, no enduser-relevance */
    double m_y2; /**< internal intermediate value storing y^2, no enduser-relevance */
    double m_z2; /**< internal intermediate value storing z^2, no enduser-relevance */
    double m_dz; /**< internal intermediate value storing distance in z, no enduser-relevance */

  }; //end class TwoHitFilters
} //end namespace Belle2
