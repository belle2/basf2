/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Modulo.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// The type of the wire ids enumerating wires within a given layer
    using IWire = signed short;

    /**
     *  This is a utility class for the free IWire type.
     *  It provides the basic methods to operate on the ISuperLayer numbers.
     */
    struct IWireUtil {
      /**
       *  Wraps the given in layer wire index by the total number of wires in this layer.
       *  Applying this function allows the advance over the numbering jumb near the
       *  wire of index zero in both directions. It brings the (incremented / decremented)
       *  wire index back to the valid range. Works for both negative and too large wire indices.
       *
       *  @param iWire   The potentially unnormalised wire index to be brought to the valid range
       *  @param nWires  The total number of wires in the layer.
       *  @return        Normalised wire index brought to the valid range.
       */
      static IWire wrappedAround(IWire iWire, IWire nWires)
      {
        return moduloFast(iWire, nWires);
      }

      /// Gets the next counter clockwise wire id properly wrapping near wire of highest index.
      static IWire getNextCCW(IWire iWire, IWire nWires)
      {
        return wrappedAround(iWire + 1, nWires);
      }

      /// Gets the next clockwise wire id properly wrapping near the wire of index zero.
      static IWire getNextCW(IWire iWire, IWire nWires)
      {
        return wrappedAround(iWire - 1, nWires);
      }
    };
  }
}
