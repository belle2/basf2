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

#include <tracking/trackFindingCDC/topology/WireNeighborKind.h>
#include <tracking/trackFindingCDC/utilities/MayBePtr.h>
#include <utility>

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCWire;

    /**
     *  Class representing a pair of neighbors in the CDC in a single layer
     *  For certain circumstances it is allowed that both wires point to nullptr,
     *  e.g. at the borders of super layers.
     *  Hence you have to check before accessing the wires.
     */
    class WireNeighborPair
      : public std::pair<MayBePtr<const CDCWire>, MayBePtr<const CDCWire> > {
    public:
      /// Type of the base class
      using Super = std::pair<MayBePtr<const CDCWire>, MayBePtr<const CDCWire> >;

      /// Constructor from the base class
      using Super::Super;

      /// Getter for the first wire
      MayBePtr<const CDCWire> getFirst() const
      { return this->first; }

      /// Getter for the second wire
      MayBePtr<const CDCWire> getSecond() const
      { return this->second; }

      /// Getter for the kind of neighbor hood relation the first wire has with the second
      WireNeighborKind getNeighborKind() const;

    };

  }
}
