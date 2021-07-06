/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
#include <cmath>

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
