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

#include <tracking/trackFindingVXD/FilterTools/SelectionVariable.h>
#include <array>
// #include <functional> // for std::reference_wrapper

namespace Belle2 {

  /**
   * SelectionVariable that is used for the Machine Learning (ML) based filters.
   *
   * Does nothing else than handing over the 3 Hits to the MLRange, where all the ML magic happens. This has to be done this way
   * since this class provides only a static function and we need different instances of ML classifiers. We do however have access
   * to the constructor of the Range classes.
   *
   * NOTE: could use std::reference_wrapper instead of bare pointers to gain some safety. This should only impact the performance
   * minorly, since std::reference_wrapper uses pointers internally and (almost) all function calls are inlined. The difference in
   * usage can be seen here and in the test-case for this SelectionVariable. For the moment keeping the pointers!
   */
  template<typename PointType>
  class MLHandover : public SelectionVariable<PointType, std::array<const PointType*, 3> > {
    // class MLHandover : public SelectionVariable<PointType, std::array<std::reference_wrapper<const PointType>, 3> > {
  public:

    static std::array<const PointType*, 3> value(const PointType& innerHit, const PointType& centerHit, const PointType& outerHit)
    // static std::array<std::reference_wrapper<const PointType>, 3>
    // value(const PointType& innerHit, const PointType& centerHit, const PointType& outerHit)
    {
      // return std::array<std::reference_wrapper<const PointType>, 3> {{ innerHit, centerHit, outerHit }};
      return std::array<const PointType*, 3> {{ &innerHit, &centerHit, &outerHit }};
    }
  };
}
