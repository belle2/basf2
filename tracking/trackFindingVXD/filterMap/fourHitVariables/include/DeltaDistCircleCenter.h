/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
#include <tracking/trackFindingVXD/filterMap/threeHitVariables/CircleCenterXY.h>
#include <framework/geometry/B2Vector3.h>
#include <math.h>

#define DELTADISTCIRCLECENTER_NAME DeltaDistCircleCenter

namespace Belle2 {

  /** calculates the distance between the estimated circle centers (using 2 subsets of given hits) in the xy-plane, returning unit: cm.
   * */
  template <typename PointType >
  class DELTADISTCIRCLECENTER_NAME : public SelectionVariable< PointType , 4, double > {
  public:
    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(DELTADISTCIRCLECENTER_NAME);



    /** calculates the distance between the estimated circle centers (using 2 subsets of given hits) in the xy-plane, returning unit: cm */
    static double value(const PointType& outerHit, const PointType& outerCenterHit, const PointType& innerCenterHit,
                        const PointType& innerHit)
    {

      B2Vector3<double> outerCircleCenter = CircleCenterXY<PointType>::value(outerHit, outerCenterHit, innerCenterHit);

      B2Vector3<double> innerCircleCenter = CircleCenterXY<PointType>::value(outerCenterHit, innerCenterHit, innerHit);

      return fabs(outerCircleCenter.Perp() - innerCircleCenter.Perp());
    } // return unit: GeV/c
  };

}
//
