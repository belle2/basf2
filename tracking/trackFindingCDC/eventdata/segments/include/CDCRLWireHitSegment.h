/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

namespace genfit {
  class TrackCand;
}

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWire;

    /// A segment consisting of two dimensional reconsturcted hits
    class CDCRLWireHitSegment :  public CDCSegment<Belle2::TrackFindingCDC::CDCRLWireHit> {
    public:
      /// Default constructor for ROOT compatibility.
      CDCRLWireHitSegment() {}

      /// Getter for the vector of wires the hits of this segment are based on in the same order
      std::vector<const Belle2::TrackFindingCDC::CDCWire*> getWireSegment() const;

    };
  }
}
