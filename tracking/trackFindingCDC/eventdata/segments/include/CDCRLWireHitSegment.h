/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment.h>

#include <vector>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCWire;
    class CDCRLWireHit;
  }
  namespace TrackFindingCDC {

    /// A segment consisting of two dimensional reconstructed hits
    class CDCRLWireHitSegment :  public CDCSegment<TrackingUtilities::CDCRLWireHit> {
    public:
      /// Default constructor for ROOT compatibility.
      CDCRLWireHitSegment() {}

      /// Getter for the vector of wires the hits of this segment are based on in the same order
      std::vector<const TrackingUtilities::CDCWire*> getWireSegment() const;
    };
  }
}
