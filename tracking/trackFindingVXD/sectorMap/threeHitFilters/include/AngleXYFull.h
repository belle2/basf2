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
#include <tracking/trackFindingVXD/sectorMap/threeHitFilters/CosAngleXY.h>
#include <tracking/trackFindingVXD/filterTools/SelectionVariableHelper.h>
#include <framework/geometry/B2Vector3.h>
#include <math.h>

namespace Belle2 {

  /** calculates the angle between the hits/vectors (XY), returning unit: angle in degrees.
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class AngleXYFull : public SelectionVariable< PointType , double > {
  public:
    /** return name of the Class */
    static const std::string name(void) {return "AngleXYFull"; };

    /** calculates the angle between the hits/vectors (XY), returning unit: angle in degrees */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, double> Helper;

      B2Vector3D outerVector(outerHit.X() - centerHit.X(), outerHit.Y() - centerHit.Y(), outerHit.Z() - centerHit.Z());
      B2Vector3D innerVector(centerHit.X() - innerHit.X(), centerHit.Y() - innerHit.Y(), centerHit.Z() - innerHit.Z());

      double result = Helper::fullAngle2D(outerVector, innerVector); // 0-pi
      //double result = CosAngleXY<PointType>::value( outerVector, innerVector); //result in rad
      result *= double(180. / M_PI); // do conversion into degree
      return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
    } // return unit: ° (0 - 180°)
  };

}
