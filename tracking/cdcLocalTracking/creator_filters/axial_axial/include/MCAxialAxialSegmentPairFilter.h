/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCAXIALAXIALSEGMENTPAIRFILTER_H_
#define MCAXIALAXIALSEGMENTPAIRFILTER_H_

#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

#include <tracking/cdcLocalTracking/fitting/CDCRiemannFitter.h>
#include <tracking/cdcLocalTracking/fitting/CDCSZFitter.h>

#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_comparison.hpp"

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    class MCAxialAxialSegmentPairFilter {

    public:

      /// Constructor
      MCAxialAxialSegmentPairFilter();

      /// Empty destructor
      ~MCAxialAxialSegmentPairFilter();

    public:

      /// Clears all remember information from the last event
      void clear();

      /// Checks if a pair of axial segments is a good combination
      CellWeight isGoodAxialAxialSegmentPair(const CDCAxialAxialSegmentPair& axialAxialSegmentPair, bool allowBackward = false);

    private:


    }; // end class MCAxialAxialSegmentPairFilter


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif // MCAXIALAXIALSEGMENTPAIRFILTER_H_
