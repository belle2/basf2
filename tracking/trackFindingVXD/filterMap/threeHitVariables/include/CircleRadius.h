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

#define CIRCLERADIUS_NAME CircleRadius
namespace Belle2 {

  /** calculates the estimation of the circle radius of the 3-hit-tracklet, returning unit: cm.
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class CIRCLERADIUS_NAME : public SelectionVariable< PointType , 3, double > {
  public:

    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(CIRCLERADIUS_NAME);


    /** calculates the estimation of the circle radius of the 3-hit-tracklet, returning unit: cm. */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {
      B2Vector3D circleCenter = CircleCenterXY<PointType>::value(outerHit, centerHit, innerHit);

      return CIRCLERADIUS_NAME<PointType>::calcAvgDistanceXY(outerHit, centerHit, innerHit, circleCenter);
    } // return unit: cm

    /** helper function with calculates the average distance in XY from the given center
    @param: a, b, c points for which the distance is evaluated
    @param: center, the center from which the relative distance is evaluated
    TODO: think about a better place to put this helper! Maybe own class but where to put the class?
    */
    static double calcAvgDistanceXY(const PointType& a, const PointType& b, const PointType& c, const B2Vector3<double>& center)
    {
      return (sqrt(std::pow(center.X() - a.X(), 2) + std::pow(center.Y() - a.Y(), 2)) +
              sqrt(std::pow(center.X() - b.X(), 2) + std::pow(center.Y() - b.Y(), 2)) +
              sqrt(std::pow(center.X() - c.X(), 2) + std::pow(center.Y() - c.Y(), 2))) / 3.;
    } // = radius in [cm], sign here not needed. normally: signKappaAB/normAB1


  };

}
