/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include "BaseAxialStereoSegmentPairNeighborChooser.h"
#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Neighbor chooser that lets all possible combinations pass.
    class AllAxialStereoSegmentPairNeighborChooser :
      public Filter<Relation<CDCAxialStereoSegmentPair>>  {

    private:
      /// Type of the super class
      typedef Filter<Relation<CDCAxialStereoSegmentPair>> Super;

    public:
      /// Importing all overloads from the super class
      using Super::operator();

    public:
      /// Implementation accepting all possible neighbors. Weight subtracts the overlap penalty.
      virtual
      NeighborWeight operator()(const CDCAxialStereoSegmentPair& fromPair,
                                const CDCAxialStereoSegmentPair& toPair) IF_NOT_CINT(override final);

    }; // end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2
