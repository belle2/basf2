/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
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
