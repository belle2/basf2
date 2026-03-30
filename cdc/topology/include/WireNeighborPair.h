/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <cdc/topology/WireNeighborKind.h>
#include <tracking/trackingUtilities/utilities/MayBePtr.h>
#include <utility>

namespace Belle2 {
  namespace CDC {

    class CDCWire;

    /**
     *  Class representing a pair of neighbors in the CDC in a single layer
     *  For certain circumstances it is allowed that both wires point to nullptr,
     *  e.g. at the borders of super layers.
     *  Hence you have to check before accessing the wires.
     */
    class WireNeighborPair
      : public std::pair<TrackingUtilities::MayBePtr<const CDCWire>, TrackingUtilities::MayBePtr<const CDCWire> > {
    public:
      /// Type of the base class
      using Super = std::pair<TrackingUtilities::MayBePtr<const CDCWire>, TrackingUtilities::MayBePtr<const CDCWire> >;

      /// Constructor from the base class
      using Super::Super;

      /// Getter for the first wire
      TrackingUtilities::MayBePtr<const CDCWire> getFirst() const
      { return this->first; }

      /// Getter for the second wire
      TrackingUtilities::MayBePtr<const CDCWire> getSecond() const
      { return this->second; }

      /// Getter for the kind of neighbor hood relation the first wire has with the second
      WireNeighborKind getNeighborKind() const;

    };

  }
}
