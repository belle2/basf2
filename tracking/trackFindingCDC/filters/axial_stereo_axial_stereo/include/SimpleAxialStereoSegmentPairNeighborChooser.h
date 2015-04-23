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

    /// Class filtering the neighborhood of axial stereo segment pairs based on simple criteria.
    class SimpleAxialStereoSegmentPairNeighborChooser :
      public Filter<Relation<CDCAxialStereoSegmentPair>> {

    private:
      /// Type of the super class
      typedef Filter<Relation<CDCAxialStereoSegmentPair>> Super;

    public:
      /// Importing all overloads from the super class
      using Super::operator();

    public:
      /// TODO implement something sensable here.
      virtual
      NeighborWeight operator()(const CDCAxialStereoSegmentPair& fromPair,
                                const CDCAxialStereoSegmentPair& toPair) IF_NOT_CINT(override final);

    }; // end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2
