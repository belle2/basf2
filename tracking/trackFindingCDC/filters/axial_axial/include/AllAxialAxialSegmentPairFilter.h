/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include "BaseAxialAxialSegmentPairFilter.h"

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter accepting all axial to axial segment pairs.
    class AllAxialAxialSegmentPairFilter  : public Filter<CDCAxialAxialSegmentPair> {

      /// Checks if a pair of axial segments is a good combination
      virtual CellWeight operator()(const CDCAxialAxialSegmentPair&) override final;

    }; // end class AllAxialAxialSegmentPairFilter

  } //end namespace TrackFindingCDC
} //end namespace Belle2
