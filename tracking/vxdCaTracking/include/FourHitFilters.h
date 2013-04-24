/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef FOURHITFILTERS_H
#define FOURHITFILTERS_H

#include <TVector3.h>
#include "ThreeHitFilters.h"



namespace Belle2 {

  /** The class 'FourHitFilters' bundles filter methods using 4 hits which are stored in TVector3s. */
  class FourHitFilters {

  public:

    /** Empty constructor. For initialisation only, an object generated this way is useless unless resetValues(a, b) is called at least once */
    FourHitFilters():
      m_radiusABC(0.),
      m_radiusBCD(0.),
      m_circleCalculated(false) {}

    /** Constructor. expects 4 hits in TVector3 format, first parameter is outer hit, second is outerCenter hit, third is innercenter hit, last one is the innermost hit */
    FourHitFilters(TVector3& outer, TVector3& outerCenter, TVector3& innerCenter, TVector3& inner):
      m_hitA(outer),
      m_hitB(outerCenter),
      m_hitC(innerCenter),
      m_hitD(inner),
      m_radiusABC(0.),
      m_radiusBCD(0.),
      m_circleCalculated(false) {}


    /** Destructor. */
    ~FourHitFilters() {}

    /** Overrides Constructor-Setup. Needed if you want to reuse the instance instead of recreating one */
    void resetValues(TVector3& outer, TVector3& outerCenter, TVector3& innerCenter, TVector3& inner) {
      m_hitA = outer;
      m_hitB = outerCenter;
      m_hitC = innerCenter;
      m_hitD = inner;
      m_radiusABC = 0.;
      m_radiusBCD = 0.;
      m_circleCalculated = false;
    }

    /** calculates dpt-value (dpt= difference in transverse momentum of 2 subsets of the hits), returning unit: GeV/c */
    double deltapT(); // TODO umbenennen! ->calcDeltapT

    /** calculates ddist2IP-value directly (ddist2IP= difference in magnitude of the points of closest approach of two circles calculated using 2 subsets of the hits) */
    double deltaDistCircleCenter();


  protected:

    /** calculates an estimation of circleCenter position including an estimation for the circle radius */
    void calcCircle();

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

#endif //FOURHITFILTERS


