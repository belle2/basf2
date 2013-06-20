/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef TWOHITFILTERS_H
#define TWOHITFILTERS_H

#include <TVector3.h>



namespace Belle2 {

  /** The class 'TwoHitFilters' bundles filter methods using 2 hits which are stored in TVector3s. */
  class TwoHitFilters {
  public:

    /** Empty constructor. For pre-inizialisation only, an object generated this way is useless unless resetValues(a, b) is called at least once */
    TwoHitFilters():
      m_x2(0.),
      m_y2(0.),
      m_z2(0.),
      m_dz(0.) {
      m_hitA.SetXYZ(0., 0., 0.);
      m_hitB.SetXYZ(0., 0., 0.);
    }

    /** Constructor. needs the first parameter is outer hit, second is inner hit. Parameters in TVector3-format*/
    TwoHitFilters(TVector3& outerHit, TVector3& innerHit):
      m_hitA(outerHit),
      m_hitB(innerHit),
      m_dz(outerHit[2] - innerHit[2]) {
      double dx = m_hitA[0] - m_hitB[0];
      double dy = m_hitA[1] - m_hitB[1];

      m_x2 = dx * dx;
      m_y2 = dy * dy;
      m_z2 = m_dz * m_dz;
    }


    /** Destructor. */
    ~TwoHitFilters() {}

    /** Overrides Constructor-Setup. Needed if you want to reuse the instance instead of recreating one */
    void resetValues(TVector3& outerHit, TVector3& innerHit) {
      m_hitA = outerHit;
      m_hitB = innerHit;
      m_dz = m_hitA[2] - m_hitB[2];

      double dx = m_hitA[0] - m_hitB[0];
      double dy = m_hitA[1] - m_hitB[1];

      m_x2 = dx * dx;
      m_y2 = dy * dy;
      m_z2 = m_dz * m_dz;
    }

    /** calculates the distance between the hits (3D), returning unit: cm^2 for speed optimization */
    double calcDist3D() { return (m_x2 + m_y2 + m_z2); } // return unit: cm^2

    /** calculates the distance between the hits (XY), returning unit: cm^2 for speed optimization */
    double calcDistXY() { return (m_x2 + m_y2); } // return unit: cm^2

    /** calculates the distance between the hits (Z only), returning unit: cm */
    double calcDistZ() { return m_dz; } // return unit: cm

    /** calculates the slope of the hits in RZ, return unit: cm (cm^2/cm = cm) */
    double calcSlopeRZ() {
      double slope = (m_x2 + m_y2) / m_dz;
      return filterNan(slope);
    } // return unit: cm  (cm^2/cm = cm)

    /** calculates the normed distance between the hits (3D), return unit: none */
    double calcNormedDist3D() {
      double normedVal = (m_x2 + m_y2) / (m_x2 + m_y2 + m_z2);
      return filterNan(normedVal);
    } // return unit: none

    /** nice little nanChecker returns 0 if value was nan, else returns value itself */
    double filterNan(double value);

  protected:

    TVector3 m_hitA; /**< outer hit (position relevant for useful filter calculation) used for the filter calculation */
    TVector3 m_hitB; /**< inner hit (position relevant for useful filter calculation) used for the filter calculation */
    double m_x2; /**< internal intermediate value storing x^2, no enduser-relevance */
    double m_y2; /**< internal intermediate value storing y^2, no enduser-relevance */
    double m_z2; /**< internal intermediate value storing z^2, no enduser-relevance */
    double m_dz; /**< internal intermediate value storing distance in z, no enduser-relevance */

  }; //end class TwoHitFilters
} //end namespace Belle2

#endif //TWOHITFILTERS


