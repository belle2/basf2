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

#include <tracking/trackFindingVXD/sectorMap/filterFramework/SelectionVariable.h>
#include <math.h>

#define SLOPERZ_NAME SlopeRZ

namespace Belle2 {

  /** This is the specialization for SpacePoints with returning floats, where value calculates the slope in R-Z for a given pair of hits.
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class SLOPERZ_NAME : public SelectionVariable< PointType , double > {
  public:
    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(SLOPERZ_NAME);


    /** value calculates the slope in R-Z for a given pair of hits. */
    static double value(const PointType& outerHit, const PointType& innerHit)
    {
      double result = atan(
                        sqrt(std::pow(double(outerHit.X() - innerHit.X()), 2)
                             + std::pow(double(outerHit.Y() - innerHit.Y()), 2)
                            ) / double(outerHit.Z() - innerHit.Z())
                      );
      // TODO: check if 0 is a good default return value in the case z_i==z_o!
      return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
    }
  };

}
