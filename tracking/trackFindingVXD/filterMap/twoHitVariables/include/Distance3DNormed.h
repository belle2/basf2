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
#include <math.h>

#define DISTANCE3DNORMED_NAME Distance3DNormed
namespace Belle2 {

  /** This is the specialization for SpacePoints with returning floats, where value calculates the normed distance between two hits in 3D */
  template <typename PointType >
  class DISTANCE3DNORMED_NAME : public SelectionVariable< PointType , 2, double > {
  public:

    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(DISTANCE3DNORMED_NAME);



    /** calculates the normed distance between the hits (3D), returning unit: none.
     *
     * Value is defined between 0-1, which is 1, if there is no distance in r^2 and is 1 if there is no distance in z
     *
     * ATTENTION: returns 0 if inf or nan is provoked (this is the behavior of the old code)
     */
    static double value(const PointType& outerHit, const PointType& innerHit)
    {

      double result =
        (std::pow(outerHit.X() - innerHit.X() , 2)
         + std::pow(outerHit.Y() - innerHit.Y() , 2))
        /
        (std::pow(outerHit.X() - innerHit.X() , 2)
         + std::pow(outerHit.Y() - innerHit.Y() , 2)
         + std::pow(outerHit.Z() - innerHit.Z() , 2));

      return
        (std::isnan(result) || std::isinf(result)) ? 0 : result;
    }


  };

}
