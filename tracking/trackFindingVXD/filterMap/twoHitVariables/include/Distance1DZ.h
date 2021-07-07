/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>

#define DISTANCE1DZ_NAME Distance1DZ

namespace Belle2 {

  /** This is the specialization for SpacePoints with returning floats, where value calculates the  distance between two hits in 1D on the Z-axis.
   *
   * (please note that this is defined for positive and negative values) */
  template <typename PointType >
  class DISTANCE1DZ_NAME : public SelectionVariable< PointType , 2 , double > {
  public:
    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(DISTANCE1DZ_NAME);

    /** calculates the distance between the hits in z (1D), returning unit: cm */
    static double value(const PointType& outerHit, const PointType& innerHit)
    {
      return outerHit.Z() - innerHit.Z();
    }
  };

}
