/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TVector3.h>
#include "tracking/vxdCaTracking/ThreeHitFilters.h"



namespace Belle2 {

  /** The class 'FourHitFilters' bundles filter methods using 4 hits which are stored in TVector3s. */
  class FourHitFilters {

  public:

    /** Empty constructor. For initialisation only, an object generated this way is useless unless resetValues(a, b) is called at least once */
    FourHitFilters():
      m_radiusABC(0.),
      m_radiusBCD(0.),
      m_circleCalculated(false) {}

    /** Constructor. expects 4 hits in TVector3 format, first parameter is outer hit, second is outerCenter hit, third is innercenter hit, last one is the innermost hit, optional parameter sets strength of magnetic field (standard is 1.5T)*/
    FourHitFilters(const TVector3& outer, const TVector3& outerCenter, const TVector3& innerCenter, const TVector3& inner,
                   const double magneticFieldStrength = 1.5):
      m_hitA(outer),
      m_hitB(outerCenter),
      m_hitC(innerCenter),
      m_hitD(inner),
      m_radiusABC(0.),
      m_radiusBCD(0.),
      m_circleCalculated(false) { m_threeHitFilter.resetMagneticField(magneticFieldStrength); }


    /** Destructor. */
    ~FourHitFilters() {}

    /** Overrides Constructor-Setup. Needed if you want to reuse the instance instead of recreating one */
    void resetValues(const TVector3& outer, const TVector3& outerCenter, const TVector3& innerCenter, const TVector3& inner)
    {
      m_hitA = outer;
      m_hitB = outerCenter;
      m_hitC = innerCenter;
      m_hitD = inner;
      m_radiusABC = 0.;
      m_radiusBCD = 0.;
      m_circleCalculated = false;
    }

    /** Overrides Constructor-Setup for magnetic field. if no value is given, magnetic field is assumed to be Belle2-Detector standard of 1.5T */
    void resetMagneticField(const double magneticFieldStrength = 1.5) { m_threeHitFilter.resetMagneticField(magneticFieldStrength); }


    /** calculates dpt-value (dpt= difference in transverse momentum of 2 subsets of the hits), returning unit: GeV/c */
    double calcDeltapT();


    /** legacy - compatibility for old cases */
    double deltapT() { return calcDeltapT(); }


    /** calculates ddist2IP-value directly (ddist2IP= difference in magnitude of the points of closest approach of two circles calculated using 2 subsets of the hits) */
    double calcDeltaDistCircleCenter();


    /** legacy - compatibility for old cases */
    double deltaDistCircleCenter() { return calcDeltaDistCircleCenter(); }
  protected:

    /** calculates an estimation of circleCenter position including an estimation for the circle radius */
    void calcCircle()
    {
      m_threeHitFilter.calcCircleCenter(m_hitA, m_hitB, m_hitC, m_centerABC);
      m_radiusABC = m_threeHitFilter.calcRadius(m_hitA, m_hitB, m_hitC, m_centerABC);
      m_threeHitFilter.calcCircleCenter(m_hitB, m_hitC, m_hitD, m_centerBCD);
      m_radiusBCD = m_threeHitFilter.calcRadius(m_hitB, m_hitC, m_hitD, m_centerBCD);

      m_circleCalculated = true;
    }

    TVector3 m_hitA; /**< outer hit (position relevant for useful filter calculation) used for the filter calculation */
    TVector3 m_hitB; /**< outerCenter hit (position relevant for useful filter calculation) used for the filter calculation */
    TVector3 m_hitC; /**< innerCenter hit (position relevant for useful filter calculation) used for the filter calculation */
    TVector3 m_hitD;  /**< inner hit (position relevant for useful filter calculation) used for the filter calculation */
    TVector3 m_centerABC;  /**< estimation of a circleCenterPosition using hits A, B & C */
    TVector3 m_centerBCD; /**< estimation of a circleCenterPosition using hitsBA, C & D */
    double m_radiusABC; /**< estimation of the radius of the circle using hits A, B & C */
    double m_radiusBCD; /**< estimation of a radius of the circle using hitsBA, C & D */
    ThreeHitFilters m_threeHitFilter;  /**< instance of ThreeHitFilters-class used for some internal calculations */
    bool m_circleCalculated; /**< initially set to false, will be set true if calcCircle() is used at least once */
  }; //end class FourHitFilters
} //end namespace Belle2
