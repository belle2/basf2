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
#include <framework/geometry/B2Vector3.h>
#include <math.h>

#define ANGLE3DSIMPLE_NAME Angle3DSimple

namespace Belle2 {

  /** calculates the angle between the hits/vectors (3D), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use Angle3DFull instead).
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class ANGLE3DSIMPLE_NAME : public SelectionVariable< PointType , 3, double > {
  public:
    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(ANGLE3DSIMPLE_NAME);

    /** calculates the angle between the hits/vectors (3D), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use Angle3DFull instead) */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {

      B2Vector3<double> outerVector(outerHit.X() - centerHit.X(), outerHit.Y() - centerHit.Y(), outerHit.Z() - centerHit.Z());
      B2Vector3<double> innerVector(centerHit.X() - innerHit.X(), centerHit.Y() - innerHit.Y(), centerHit.Z() - innerHit.Z());

      // fullCalc would be acos(m_vecAB.Dot(m_vecBC) / m_vecAB.Mag()*m_vecBC.Mag()), but here time-consuming parts have been neglected
      double result = outerVector.Dot(innerVector) / (outerVector.Mag2() * innerVector.Mag2());
      return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
    }
  };

}
