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

#include <tracking/trackFindingCDC/hough/Box.h>
#include <tracking/trackFindingCDC/numerics/Sign.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Function to add a new dimension to a box
     *  It *sweeps* the original box over a finite range
     *  in the new dimension
     */
    template<class NewType, class Box>
    class SweepBoxImpl;

    /// Actual implementation pasting together the new dimension with the original box.
    template<class NewType, class... OldTypes>
    struct SweepBoxImpl<NewType, Box<OldTypes...> > {
      typedef Box<NewType, OldTypes...> Type;
    };

    /// Short hand for SweepBoxImpl
    template<class NewType, class Box>
    using SweepBox = typename SweepBoxImpl<NewType, Box>::Type;

  } // end namespace TrackFindingCDC
} // end namespace Belle2
