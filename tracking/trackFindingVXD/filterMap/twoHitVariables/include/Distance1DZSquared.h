/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>

#define DISTANCE1DZSQUARED_NAME Distance1DZSquared

namespace Belle2 {

  /** This is the specialization for SpacePoints with returning floats, where value calculates the squared distance between two hits in 1D on the Z-axis */
  template <typename PointType >
  class DISTANCE1DZSQUARED_NAME : public SelectionVariable< PointType , 2, double > {
  public:
    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class (CLASS_NAME)*/
    PUT_NAME_FUNCTION(DISTANCE1DZSQUARED_NAME);

    /** calculates the squared distance between the hits in Z (in 1D), returning unit: cm^2 */
    static double value(const PointType& outerHit, const PointType& innerHit)
    {
      return std::pow(double(outerHit.Z() - innerHit.Z()), 2);
    }
  };

}
