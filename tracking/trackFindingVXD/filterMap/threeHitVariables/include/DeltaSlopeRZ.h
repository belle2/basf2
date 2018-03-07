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
#include <tracking/trackFindingVXD/filterMap/twoHitVariables/SlopeRZ.h>

#include <framework/geometry/B2Vector3.h>

#include <math.h>

#define DELTASLOPERZ_NAME DeltaSlopeRZ

namespace Belle2 {

  /** calculates deviations in the slope of the inner segment and the outer segment, returning unit: none
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class DELTASLOPERZ_NAME : public SelectionVariable< PointType , 3, double > {
  public:

    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(DELTASLOPERZ_NAME);

    /** calculates deviations in the slope of the inner segment and the outer segment, returning unit: none */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {

      double slopeOC = SlopeRZ<PointType>::value(outerHit, centerHit);
      double slopeCI = SlopeRZ<PointType>::value(centerHit, innerHit);

      return slopeCI - slopeOC;
    } // return unit: none
  };

}
