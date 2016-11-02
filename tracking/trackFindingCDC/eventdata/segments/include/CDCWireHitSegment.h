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

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHit;

    /// A segment consisting of two dimensional reconsturcted hits
    class CDCWireHitSegment :  public CDCSegment<const Belle2::TrackFindingCDC::CDCWireHit*> {

    public:
      /// Getter for the vector of wires the hits of this segment are based on in the same order
      std::vector<const Belle2::TrackFindingCDC::CDCWire*> getWireSegment() const;

    };

  }
}
