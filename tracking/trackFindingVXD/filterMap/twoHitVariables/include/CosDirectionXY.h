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

#define COSDIRECTIONXY_NAME CosDirectionXY

namespace Belle2 {

  /** This is a specialization returning floats, where value calculates the  cos of the angle of the segment of two hits
      in the XY plane */
  template <typename PointType >
  class COSDIRECTIONXY_NAME : public SelectionVariable< PointType , 2, double > {
  public:
    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(COSDIRECTIONXY_NAME);

    /** calculates the distance between the hits in z (1D), returning unit: cm */
    static double value(const PointType& outerHit, const PointType& innerHit)
    {
      double result = (outerHit.X() * innerHit.X() + outerHit.Y() * innerHit.Y());
      result /= sqrt(outerHit.X() * outerHit.X() + outerHit.Y() * outerHit.Y());
      result /= sqrt(innerHit.X() * innerHit.X() + innerHit.Y() * innerHit.Y());
      return result;
    }
  };

}
