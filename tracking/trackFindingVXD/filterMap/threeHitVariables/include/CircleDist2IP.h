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
#include <tracking/trackFindingVXD/filterMap/threeHitVariables/CircleRadius.h>
#include <framework/geometry/B2Vector3.h>
#include <math.h>

#define CIRCLEDIST2IP_NAME CircleDist2IP

namespace Belle2 {

  /** calculates the distance of the point of closest approach of circle to the IP, returning unit: cm
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class CIRCLEDIST2IP_NAME : public SelectionVariable< PointType , 3, double > {
  public:

    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(CIRCLEDIST2IP_NAME);

    /** calculates the distance of the point of closest approach of circle to the IP, returning unit: cm */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {

      B2Vector3D cCenter = CircleCenterXY<PointType>::value(outerHit, centerHit, innerHit);
      double circleRadius = CircleRadius<PointType>::calcAvgDistanceXY(outerHit, centerHit, innerHit, cCenter);

      // distance of closest approach of circle to the IP :
      // WARNING only valid for IP=0,0,X
      return (fabs(cCenter.Perp() - circleRadius));
    } // return unit: cm
  };

}
