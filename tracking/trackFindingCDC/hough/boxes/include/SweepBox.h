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

#include <tracking/trackFindingCDC/hough/boxes/Box.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Function to add a new dimension to a box
     *  It *sweeps* the original box over a finite range
     *  in the new dimension
     */
    template<class ANewType, class ABox>
    struct SweepBoxImpl;

    /// Actual implementation pasting together the new dimension with the original box.
    template<class ANewType, class... AOldTypes>
    struct SweepBoxImpl<ANewType, ABox<AOldTypes...> > {
      /// The type of the underlaying box.
      using Type = ABox<ANewType, AOldTypes...>;
    };

    /// Short hand for SweepBoxImpl
    template<class ANewType, class ABox>
    using SweepBox = typename SweepBoxImpl<ANewType, ABox>::Type;

  }
}
