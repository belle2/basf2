/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
#include <array>

#define MLHANDOVER_NAME MLHandover

namespace Belle2 {

  /**
   * SelectionVariable that is used for the Machine Learning (ML) based filters.
   *
   * Does nothing else than handing over the 3x3 git coordinates to the MLRange, where all the ML magic happens. This has to be done
   * this way since this class provides only a static function and we need different instances of ML classifiers. We do however have
   * access to the constructor of the Range classes.
   */
  template<typename PointType, size_t Ndims = 9>
  class MLHANDOVER_NAME : public SelectionVariable<PointType, 3, std::array<double, Ndims> > {

  public:

    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(MLHANDOVER_NAME);

    /** Getter for coordinate values of the three hits used in a three hit filter
     * @param innerHit : the inner hit
     * @param centerHit : the center hit
     * @param outerHit : the outer hit
     * @return array containing the coordinates of the three hits in the order x, y, z for inner, x, y, y for center, outer
     */
    static std::array<double, Ndims> value(const PointType& innerHit, const PointType& centerHit, const PointType& outerHit)
    {
      return std::array<double, Ndims> {{
          innerHit.X(), innerHit.Y(), innerHit.Z(),
          centerHit.X(), centerHit.Y(), centerHit.Z(),
          outerHit.X(), outerHit.Y(), outerHit.Z()
        }
      };
    }
  };
}
