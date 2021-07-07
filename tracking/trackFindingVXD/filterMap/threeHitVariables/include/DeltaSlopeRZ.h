/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
#include <tracking/trackFindingVXD/filterMap/twoHitVariables/SlopeRZ.h>

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
