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

#include <tracking/trackFindingVXD/FilterTools/SelectionVariable.h>
#include <tracking/trackFindingVXD/FilterTools/SelectionVariableHelper.h>
#include <math.h>

namespace Belle2 {

  /** This is the specialization for SpacePoints with returning floats, where value calculates the  slope in R-Z for a given pair of hits.
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class SlopeRZ : public SelectionVariable< PointType , float > {
  public:

    /** value calculates the slope in R-Z for a given pair of hits. */
    static float value(const PointType& outerHit, const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, float> Helper;

      return Helper::calcSlopeRZ(outerHit, innerHit);
    }
  };

}
