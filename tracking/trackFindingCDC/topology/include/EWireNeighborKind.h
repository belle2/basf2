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

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Namespace hiding the constants of the enum
    namespace NWireNeighborKind {
      /// Type for the neighbor relationship from in wire to an other, imagined in the clock
      enum class EWireNeighborKind : signed short {

        /// Constant to indicate that two wires are not neighbors.
        c_None = 0,

        /// Constant for clockwise outwards
        c_CWOut = 1,

        /// Constant for clockwise
        c_CW = 3,

        /// Constant for clockwise inwards
        c_CWIn = 5,

        /// Constant for counterclockwise inwards
        c_CCWIn = 7,

        /// Constant for counterclockwise
        c_CCW = 9,

        /// Constant for counterclockwise outwards
        c_CCWOut = 11,
      };
    }

    /// Importing only the enum but not the constants
    using EWireNeighborKind = NWireNeighborKind::EWireNeighborKind;

  }

}

