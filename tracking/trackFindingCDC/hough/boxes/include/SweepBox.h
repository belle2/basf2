/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
