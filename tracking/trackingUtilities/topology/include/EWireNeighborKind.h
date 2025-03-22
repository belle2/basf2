/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

